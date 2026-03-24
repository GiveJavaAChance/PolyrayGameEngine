#include <iostream>
#include "base/glad/glad.h"
#include "Allocator.h"
#include "BindingRegistry.h"
#include "BindSet.h"
#include "BitSet.h"
#include "BVH.h"
#include "CascadedQueue.h"
#include "DynamicArray.h"
#include "ECS.h"
#include "EventBus.h"
#include "FFT.h"
#include "FullscreenQuad.h"
#include "GLFramebuffer.h"
#include "GLFramebufferMSAA.h"
#include "GLFWindow.h"
#include "GLGbuffer.h"
#include "GLGbufferMSAA.h"
#include "GLTexture.h"
#include "GLTextureMSAA.h"
#include "HeapBuffer.h"
#include "IDGenerator.h"
#include "Input.h"
#include "InputSystem.h"
#include "InstancedRenderSystem.h"
#include "MultiDynamicArray.h"
#include "perf.h"
#include "Physics2D.h"
#include "Physics3D.h"
#include "prvl.h"
#include "Registry.h"
#include "RenderObject.h"
#include "ResourceManager.h"
#include "ScriptSystem.h"
#include "ShaderBuffer.h"
#include "ShaderBufferHeap.h"
#include "ShaderManager.h"
#include "ShaderPreprocessor.h"
#include "ShaderProgram.h"
#include "StablePool.h"
#include "StaticRegistry.h"
#include "stb_image_ex.h"
#include "TextRenderer.h"
#include "Time.h"
#include "typereg.h"
#include "UnorderedRegistry.h"

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

ECS ecs;
EventBus eventBus;

Camera3D cam;
Environment env;

#define MODE 1

#if MODE == 0
struct Vertex3D {
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

struct Instance3D {
    mat4 instanceTransform;
};

struct MyScript : Script {
    double t = 0.0d;

    void frameUpdate(double dt) {
        t += dt;
        float s = 1.0f * (float) sin(t);
        if(mat4* t = getComponentPtr<mat4>()) {
            mat4& tx = *t;
            mat3 rot = mat3(vec3(tx[0]), vec3(tx[1]), vec3(tx[2]));
            rot = rotateX(s) * rotateY(s) * rotateZ(s);
            tx = mat4(vec4(rot[0], 0.0f), vec4(rot[1], 0.0f), vec4(rot[2], 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
    }
};

InstancedRenderSystem<mat4>* renderSystem;
ScriptSystem<MyScript>* scriptSystem;

int main() {
    ecs.registerComponentType<mat4, Layout::AoS>();
    ResourceManager::addLocalResource("res/shaders");
    ResourceManager::addLocalResource("res/textures");

    renderSystem = new InstancedRenderSystem<mat4>(&ecs);
    scriptSystem = new ScriptSystem<MyScript>(&ecs);

    GLFWindow w("testing");
    w.createFrame(500, 500, false, true, false);

    cam.projection = reverseZPerspectiveProjection(1.57079632679f, static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight()), 0.1f);
    cam.inverseProjection = inverse(cam.projection);

    ShaderBuffer cameraBuffer(GL_DYNAMIC_DRAW);
    ShaderBuffer envBuffer(GL_DYNAMIC_DRAW);
    cameraBuffer.setSize(sizeof(Camera3D));
    envBuffer.setSize(sizeof(Environment));

    env.sunColor = vec3(1.0f, 1.0f, 0.5f);
    env.ambientColor = vec3(0.2f, 0.5f, 1.0f);
    env.sunDir = normalize(vec3(1.0f));

    envBuffer.uploadPartialData(&env, 1, 0);

    ShaderManager::setValue("CAM3D_IDX", BindingRegistry::bindBufferBase(cameraBuffer, GL_UNIFORM_BUFFER));
    ShaderManager::setValue("ENV_IDX", BindingRegistry::bindBufferBase(envBuffer, GL_UNIFORM_BUFFER));

    GLuint vertexShader = ShaderManager::compileShaderFile("Texture3D.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = ShaderManager::compileShaderFile("Texture3D.frag", GL_FRAGMENT_SHADER);

    ShaderProgram shader = ShaderManager::createProgram({vertexShader, fragmentShader});

    shader.use();
    shader.setUniform("metallic", 0.5f);
    shader.setUniform("roughness", 0.5f);
    shader.setUniform("F0", 0.05f, 0.05f, 0.05f);

    //GLTexture tex = ResourceManager::getResourceAsTexture("anim.png");
    GLTexture tex = ResourceManager::getResourceAsTexture("Placeholder Textures.png");
    //GLTexture tex = GLTexture::createTexture2D(1, 1);
    //uint32_t pixel = 0xFFFFFFFFu;
    //tex.set2DTextureData(&pixel, 1, 1, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    RenderObject obj{shader, &tex};
    obj.mode = GL_TRIANGLE_STRIP;

    Vertex3D vertices[]{
        {{ 10.0f, 0.0f, -10.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-10.0f, 0.0f, -10.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 10.0f, 0.0f,  10.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{-10.0f, 0.0f,  10.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}
    };
    obj.uploadVertices(vertices, 4);

    uint32_t objectID = renderSystem->addObject(&obj);

    Entity e = ecs.createEntity();
    e.addComponent(diag(vec4(1.0)));
    e.addComponent(RenderInstance<mat4>(objectID));
    e.addComponent(MyScript{});

    Entity e2 = ecs.createEntity();
    e2.addComponent(mat4(
        vec4( 0.0,  1.0, 0.0,  0.0),
        vec4(-1.0,  0.0, 0.0,  0.0),
        vec4( 0.0,  0.0, 1.0,  0.0),
        vec4(10.0, 10.0, 0.0,  1.0)
    ));
    e2.addComponent(RenderInstance<mat4>(objectID));

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0d);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);

    //mat3 preRot = rotateX(0.01f) * rotateY(0.01f);
    //mat3 rot = rotateZ(0.01f);

    ecs.setup();

    float t = 0.0f;
    double dt = 1.0d / 165.0d;
    while(w.isWindowOpen()) {
        uint64_t startTime = Time::nanoTime();
        if(t == 0.0f || Input::getKey(GLFW_KEY_SPACE)) {
            cam.cameraPos = vec3(0.0f, cosf(t * 0.01f) * 2.0f + 3.0f, -3.0f) * rotateY(-t * 0.01f);
            mat3 newTx = rotateZ(sinf(t * 0.1f) * 0.025f + sinf(t * 0.23473f) * 0.0125f) * lookat(cam.cameraPos, vec3(0.0f, sinf(t * 0.1f) * 0.1f, 0.0f));
            cam.cameraTransform = mat4(vec4(newTx[0], 0.0f), vec4(newTx[1], 0.0f), vec4(newTx[2], 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));
            cam.inverseCameraTransform = inverse(cam.cameraTransform);
            cameraBuffer.uploadPartialData(&cam, 1, 0);
            t++;
        }

        ecs.update(dt);

        std::cout << (Time::nanoTime() - startTime) << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render(obj);
        w.update();
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0d;
    }
}
#elif MODE == 1

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
    return vec4(planeNormal, (planeHeight - dot(pos, planeNormal)) / dot(dir, planeNormal));
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
        return vec4(-1.0f);
    }
    int side = (tNear == tmin.x) ? 0 : (tNear == tmin.y) ? 1 : 2;
    vec3 n = vec3(0.0f);
    n[side] = (dir[side] > 0.0f) ? -1.0f : 1.0f;
    return vec4(n, tNear);
}

vec4 bounds(const vec3& pos, const vec3& dir, float r) {
    float p0 = pos.x * dir.x + pos.z * dir.z;
    float pp = p0 * p0 + r * r - pos.x * pos.x - pos.z * pos.z;
    if(pp < 0.0f) {
        return vec4(-1.0f);
    }
    float p1 = sqrt(pp);
    float t0 = p1 - p0;
    float t1 = -p1 - p0;
    if(t0 < 0.0f && t1 < 0.0f) {
        return vec4(-1.0f);
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
    return vec4(n, t);
}

vec4 rayWorld(const vec3& pos, const vec3& dir) {
    return select(
        select(
            select(
                plane(pos, dir, vec3(0.0f, 1.0f, 0.0f), 0.0f),
                bounds(pos, dir, 15.0f)
            ),
            plane(pos, dir, vec3(0.0f, 1.0f, 0.0f), 4.0f)
        ),
        select(
            aabb(pos, dir, vec3(5.0f, 0.0f, 5.0f), vec3(7.0f, 1.0f, 7.0f)),
            aabb(pos, dir, vec3(5.0f, 0.0f, -7.0f), vec3(7.0f, 1.0f, -5.0f))
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

    GLFWindow w("testing");
    w.createFrame(500, 500, false, true, false);

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0d);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    cam.projection = reverseZPerspectiveProjection(1.57079632679f, static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight()), 0.1f);
    cam.inverseProjection = inverse(cam.projection);

    cam.cameraTransform = diag(vec4(1.0));
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

    dvec2 ang = dvec2(0.0);

    uint32_t idx = 0u;

    double dt = 1.0d / 165.0d;
    while(w.isWindowOpen()) {
        uint64_t startTime = Time::nanoTime();

        dvec2 size = dvec2(w.getWidth(), w.getHeight());
        dvec2 center = 0.5 * size;
        dvec2 p;
        w.getMousePos(p.x, p.y);
        w.setMousePos(center.x, center.y);
        dvec2 d = (p - center) * 2.0f;
        ang += dvec2(d.y, -d.x) / size;
        cam.cameraTransform = mat4(rotateX(ang.x) * rotateY(ang.y));
        cam.inverseCameraTransform = transpose(cam.cameraTransform);

        vec3 forward = -vec3(cam.inverseCameraTransform[2]);
        forward.y = 0.0f;
        forward = normalize(forward);
        vec3 right = vec3(cam.inverseCameraTransform[0]);
        right.y = 0.0f;
        right = normalize(right);

        vec3 movement = vec3(0.0);
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
                vec3 dir = -normalize(s * vec3(cam.inverseCameraTransform[0]) + c * vec3(cam.inverseCameraTransform[1]) + vec3(cam.inverseCameraTransform[2]));
                vec3 o = cam.cameraPos + (vec3(cam.inverseCameraTransform[0]) - vec3(cam.inverseCameraTransform[1]) - vec3(cam.inverseCameraTransform[2])) * 0.5f;
                vec4 hit = rayWorld(o, dir);
                if(hit.w >= 0.0f && hit.w <= 10.0f) {
                    vec3 pos = o + dir * hit.w;
                    vec4 p = vec4(pos, encodeNormal(vec3(hit)));
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
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0d;
    }
}
#endif
