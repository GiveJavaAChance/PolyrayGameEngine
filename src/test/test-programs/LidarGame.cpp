#include <cstdint>
#include <iostream>

#include <base/glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLFWindow.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>
#include <rendering/ShaderBuffer.h>
#include <rendering/GLFramebuffer.h>
#include <BindingRegistry.h>
#include <utils/perf.h>
#include <input/Input.h>
#include <RenderObject.h>
#include <ecs/ECS.h>

#include <FullscreenQuad.h>

#include <physics/3d/Physics3D.h>
#include <InstancedRenderSystem.h>
#include <ScriptSystem.h>

struct Camera3D {
    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    mat4 projection;
    mat4 inverseProjection;
    vec3 cameraPos;
    float __padding__0;
};

struct Environment {
    vec3 sunDir;
    float __padding__0;
    vec3 sunColor;
    float __padding__1;
    vec3 ambientColor;
    float __padding__2;
};

Camera3D cam;
Environment env;

GLFWindow w("testing");

void render(const RenderObject& obj) {
    if(obj.vertexCount == 0u || obj.instanceCount == 0u) {
        return;
    }
    obj.shader.use();
    if(obj.texture) {
        glBindTextureUnit(0, obj.texture->ID);
    }
    glBindVertexArray(obj.vao);
    glDrawArraysInstanced(obj.mode, 0, obj.vertexCount, obj.instanceCount);
}

ECS ecs;
EventBus eventBus;

bool mouseDown = false;
float focus = 0.5f;

inline vec4 select(const vec4& t0, const vec4& t1) {
    if(t0.w < 0.0f) {
        return t1;
    }
    if(t1.w < 0.0f) {
        return t0;
    }
    if(t0.w < t1.w) {
        return t0;
    }
    return t1;
}

vec4 plane(const vec3& pos, const vec3& dir, const vec3& planeNormal, float planeHeight) {
    return prvl::vec4(planeNormal, (planeHeight - dot(pos, planeNormal)) / dot(dir, planeNormal));
}

vec4 aabb(const vec3& pos, const vec3& dir, const vec3& bMin, const vec3& bMax) {
    vec3 invDir = 1.0f / dir;
    vec3 t0s = (bMin - pos) * invDir;
    vec3 t1s = (bMax - pos) * invDir;

    vec3 tmin = min(t0s, t1s);
    vec3 tmax = max(t0s, t1s);

    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);

    if(tNear > tFar || tFar < 0.0f) {
        return prvl::vec4(-1.0f);
    }
    int side = (tNear == tmin.x) ? 0 : (tNear == tmin.y) ? 1 : 2;
    vec3 n = prvl::vec3(0.0f);
    n[side] = (dir[side] > 0.0f) ? -1.0f : 1.0f;
    return prvl::vec4(n, tNear);
}

vec4 bounds(const vec3& pos, const vec3& dir, float r) {
    float p0 = pos.x * dir.x + pos.z * dir.z;
    float pp = p0 * p0 + r * r - pos.x * pos.x - pos.z * pos.z;
    if(pp < 0.0f) {
        return prvl::vec4(-1.0f);
    }
    float p1 = sqrt(pp);
    float t0 = p1 - p0;
    float t1 = -p1 - p0;
    if(t0 < 0.0f && t1 < 0.0f) {
        return prvl::vec4(-1.0f);
    }
    float t;
    if(t0 < 0.0f) {
        t = t1;
    } else if(t1 < 0.0f) {
        t = t0;
    } else {
        t = min(t0, t1);
    }
    vec3 n = -pos - dir * t;
    n.y = 0.0f;
    n = normalize(n);
    return prvl::vec4(n, t);
}

vec4 rayWorld(const vec3& pos, const vec3& dir) {
    return select(
        select(
            select(
                plane(pos, dir, prvl::vec3(0.0f, 1.0f, 0.0f), 0.0f),
                bounds(pos, dir, 15.0f)
            ),
            plane(pos, dir, prvl::vec3(0.0f, 1.0f, 0.0f), 4.0f)
        ),
        select(
            aabb(pos, dir, prvl::vec3(5.0f, 0.0f, 5.0f), prvl::vec3(7.0f, 1.0f, 7.0f)),
            aabb(pos, dir, prvl::vec3(5.0f, 0.0f, -7.0f), prvl::vec3(7.0f, 1.0f, -5.0f))
        )
    );
}

inline float encodeNormal(const vec3& normal) {
    int a = clamp(static_cast<int>(std::acos(clamp(normal.y, -0.999f, 0.999f)) / 3.14159265359f * 1024.0f), 0, 1023);
    int b = clamp(static_cast<int>(std::atan2(normal.z, normal.x) / 6.28318530718f * 1024.0f + 512.0f), 0, 1023);
    return static_cast<float>(b << 10 | a);
}

float r() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

struct Line {
    vec3 a;
    vec3 b;
};

struct RayEmitter {
    ShaderProgram shader;
    ShaderBuffer lineVbo;
    ShaderBuffer alphaBuffer;
    GLuint vao;
    bool linesUpdated = false;
    uint32_t count = 0u;
    DynamicArray<Line> rayLines;
    DynamicArray<float> rayAlphas;

    RayEmitter(const ShaderBuffer& alphaBuffer) : shader(ShaderManager::createProgram({ShaderManager::compileShaderFile("lidar line.vert", GL_VERTEX_SHADER), ShaderManager::compileShaderFile("lidar line.frag", GL_FRAGMENT_SHADER)})), lineVbo(GL_DYNAMIC_DRAW), alphaBuffer(alphaBuffer), vao(ShaderManager::createVAO(shader, {lineVbo.ID})) {
    }

    void add(const vec3& a, const vec3& b) {
        rayLines.emplace(a, b);
        rayAlphas.add(1.0f);
        count++;
        linesUpdated = true;
    }

    void update(double dt) {
        for(uint32_t i = 0u; i < count;) {
            float& v = rayAlphas[i];
            v -= dt * 10.0f;
            if(v < 0.0f) {
                rayLines[i] = rayLines[count - 1u];
                rayAlphas[i] = rayAlphas[count - 1u];
                rayLines.removeEnd(1u);
                rayAlphas.removeEnd(1u);
                count--;
                linesUpdated = true;
            } else {
                i++;
            }
        }
        if(linesUpdated) {
            linesUpdated = false;
            lineVbo.uploadData<Line>(rayLines.data(), count);
        }
        alphaBuffer.uploadData<float>(rayAlphas.data(), count);
    }

    void render() {
        shader.use();
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, count * 2);
    }
};

int main() {
    ResourceManager::addLocalResource("res/shaders");

    w.createFrame(500, 500, false, true, false);

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    cam.projection = reverseZPerspectiveProjection(1.57079632679f, static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight()), 0.1f);
    cam.inverseProjection = inverse(cam.projection);

    cam.cameraTransform = diag(prvl::vec4(1.0));
    cam.inverseCameraTransform = inverse(cam.cameraTransform);

    ShaderBuffer cameraBuffer(GL_DYNAMIC_DRAW);
    cameraBuffer.setSize(sizeof(Camera3D));
    cameraBuffer.uploadPartialData(&cam, 1, 0);

    ShaderManager::setValue("CAM3D_IDX", BindingRegistry::bindBufferBase(cameraBuffer, GL_UNIFORM_BUFFER));

    GLuint vertexShader = ShaderManager::compileShaderFile("lidar.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = ShaderManager::compileShaderFile("lidar.frag", GL_FRAGMENT_SHADER);

    ShaderProgram shader = ShaderManager::createProgram({vertexShader, fragmentShader});

    RenderObject obj{shader, nullptr};
    obj.mode = GL_TRIANGLE_STRIP;

    vec2 vertices[]{
        {0.0f, 0.05f * 1.85f},
        { 0.05f * 1.85f, -0.05f},
        {-0.05f * 1.85f, -0.05f}
    };
    obj.uploadVertices(vertices, 3);

    ShaderBuffer alphaBuffer(GL_DYNAMIC_DRAW);
    ShaderManager::setValue("ALPHA_IDX", BindingRegistry::bindBufferBase(alphaBuffer, GL_SHADER_STORAGE_BUFFER));
    RayEmitter emitter{alphaBuffer};

    constexpr uint32_t count = 1000000u;
    obj.instanceVbo.setSize(count * sizeof(vec4));

    cam.cameraPos.y = 2.0f;

    ecs.setup();

    GLFramebuffer framebuffer(w.getWidth(), w.getHeight());
    FullscreenQuad quad;
    quad.setTexture(&framebuffer.color);

    w.mousePress = [](float x, float y, int button) {
        mouseDown = true;
    };
    w.mouseRelease = [](float x, float y, int button) {
        mouseDown = false;
    };
    w.scroll = [](float x, float y, float amt) {
        focus *= 1.0f - amt * 0.1f;
        focus = clamp(focus, 0.1f, 1.0f);
    };

    dvec2 ang = prvl::dvec2(0.0);

    uint32_t idx = 0u;

    double dt = 1.0 / 165.0;
    while(w.isWindowOpen()) {
        uint64_t startTime = Time::nanoTime();

        dvec2 size = prvl::dvec2(w.getWidth(), w.getHeight());
        dvec2 center = 0.5 * size;
        dvec2 p;
        w.getMousePos(p.x, p.y);
        w.setMousePos(center.x, center.y);
        dvec2 d = (p - center) * 2.0f;
        ang += prvl::dvec2(d.y, -d.x) / size;
        cam.cameraTransform = mat4(rotateX(ang.x) * rotateY(ang.y));
        cam.inverseCameraTransform = transpose(cam.cameraTransform);

        vec3 forward = -prvl::vec3(cam.inverseCameraTransform[2]);
        forward.y = 0.0f;
        forward = normalize(forward);
        vec3 right = prvl::vec3(cam.inverseCameraTransform[0]);
        right.y = 0.0f;
        right = normalize(right);

        vec3 movement = prvl::vec3();
        if(Input::getKey(GLFW_KEY_W)) {
            movement += forward;
        }
        if(Input::getKey(GLFW_KEY_S)) {
            movement -= forward;
        }
        if(Input::getKey(GLFW_KEY_D)) {
            movement += right;
        }
        if(Input::getKey(GLFW_KEY_A)) {
            movement -= right;
        }
        float len = length(movement);
        if(len > 0.0f) {
            float mul = dt * (Input::getKey(GLFW_KEY_LEFT_SHIFT) ? 4.0f : 2.0f) / len;
            cam.cameraPos += movement * mul;
        }
        if(mouseDown) {
            for(uint32_t i = 0u; i < 1000u; i++) {
                float a = 6.28318530718f * r();
                float rf = sqrt(r()) * focus;
                float s = sin(a) * rf;
                float c = cos(a) * rf;
                vec3 dir = -normalize(s * prvl::vec3(cam.inverseCameraTransform[0]) + c * prvl::vec3(cam.inverseCameraTransform[1]) + prvl::vec3(cam.inverseCameraTransform[2]));
                vec3 o = cam.cameraPos + (prvl::vec3(cam.inverseCameraTransform[0]) - prvl::vec3(cam.inverseCameraTransform[1]) - prvl::vec3(cam.inverseCameraTransform[2])) * 0.5f;
                vec4 hit = rayWorld(o, dir);
                if(hit.w >= 0.0f && hit.w <= 10.0f) {
                    vec3 pos = o + dir * hit.w;
                    vec4 p = prvl::vec4(pos, encodeNormal(prvl::vec3(hit)));
                    obj.instanceVbo.uploadPartialData(&p, 1, idx++);
                    obj.instanceCount = max(obj.instanceCount, idx);
                    emitter.add(o, pos);
                    if(idx == 1000000u) {
                        idx = 0u;
                    }
                }
            }
        }

        emitter.update(dt);

        ecs.update(dt);

        cameraBuffer.uploadPartialData(&cam, 1, 0);

        framebuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render(obj);
        glEnable(GL_DEPTH_TEST);
        emitter.render();

        framebuffer.unbind();

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        quad.render();

        w.update();
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
    }
}
