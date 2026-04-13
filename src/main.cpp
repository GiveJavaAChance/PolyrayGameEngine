#include <iostream>

#include <base/glad/glad.h>

#include <input/BindSet.h>
#include <input/Input.h>
#include <input/InputSystem.h>

#include <physics/2d/Physics2D.h>
#include <physics/3d/Physics3D.h>

#include <rendering/GLFramebuffer.h>
#include <rendering/GLFramebufferMSAA.h>
#include <rendering/GLGbuffer.h>
#include <rendering/GLGbufferMSAA.h>
#include <rendering/GLTexture.h>
#include <rendering/GLTextureMSAA.h>
#include <rendering/ShaderBuffer.h>
#include <rendering/ShaderBufferHeap.h>

#include <scene/2d/Node2D.h>
#include <scene/2d/SceneNode2D.h>
#include <scene/2d/Scene2D.h>

#include <scene/3d/Node3D.h>
#include <scene/3d/SceneNode3D.h>
#include <scene/3d/Scene3D.h>

#include <shader/ShaderManager.h>
#include <shader/ShaderPreprocessor.h>
#include <shader/ShaderProgram.h>

#include <structure/BitSet.h>
#include <structure/BVH.h>
#include <structure/CascadedQueue.h>
#include <structure/DynamicArray.h>
#include <structure/HeapBuffer.h>
#include <structure/IDGenerator.h>
#include <structure/MultiDynamicArray.h>
#include <structure/Registry.h>
#include <structure/StablePool.h>
#include <structure/StaticRegistry.h>
#include <structure/UnorderedRegistry.h>

#include <utils/FFT.h>
#include <utils/perf.h>

#include <Allocator.h>
#include <BindingRegistry.h>
#include <EventBus.h>
#include <FullscreenQuad.h>
#include <GLFWindow.h>
#include <prvl.h>
#include <RenderObject.h>
#include <ResourceManager.h>
#include <TextRenderer.h>
#include <typereg.h>

#include <InstancedRenderSystem.h>
#include <ScriptSystem.h>

#include <ecs/Component.h>
#include <ecs/ComponentRef.h>
#include <ecs/ECS.h>
#include <ecs/Storage.h>

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

#define MODE 0

#if MODE == 0
struct Vertex3D {
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

struct MyScript : Script {

    void physicsUpdate(double dt) {
        PhysicsObject3D obj;
        if(getComponent<PhysicsObject3D>(obj)) {
            obj.accY = -3.0;
            setComponent(obj);
        }
    }

    void frameUpdate(double dt) {
        mat4* t = getComponentPtr<mat4>();
        PhysicsObject3D obj;
        if(t && getComponent<PhysicsObject3D>(obj)) {
            mat4& tx = *t;
            vec4& tr = tx[3];
            tr.x = static_cast<float>(obj.posX);
            tr.y = static_cast<float>(obj.posY);
            tr.z = static_cast<float>(obj.posZ);
        }
    }
};

InstancedRenderSystem<mat4>* renderSystem;
ScriptSystem<MyScript>* scriptSystem;
Physics3D* physicsSystem;
Scene3D* scene;

struct AABB {
};
struct Sphere {
};

bool collideAABB_AABB(Collider3D& a, AABB* aData, Collider3D& b, AABB* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;
    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;
    double overlapX = halfAx + halfBx - abs(dx);
    double overlapY = halfAy + halfBy - abs(dy);
    double overlapZ = halfAz + halfBz - abs(dz);
    if (overlapX <= 0.0 || overlapY <= 0.0 || overlapZ <= 0.0) {
        return false;
    }
    if(overlapX < overlapY && overlapX < overlapZ) {
        out.collisionNormalX = (dx < 0.0) ? -1.0 : 1.0;
        out.collisionNormalY = 0.0;
        out.collisionNormalZ = 0.0;
        out.penetrationDepth = overlapX;
    } else if(overlapY < overlapZ) {
        out.collisionNormalX = 0.0;
        out.collisionNormalY = (dy < 0.0) ? -1.0 : 1.0;
        out.collisionNormalZ = 0.0;
        out.penetrationDepth = overlapY;
    } else {
        out.collisionNormalX = 0.0;
        out.collisionNormalY = 0.0;
        out.collisionNormalZ = (dz < 0.0) ? -1.0 : 1.0;
        out.penetrationDepth = overlapZ;
    }
    return true;
}

bool collideSphere_Sphere(Collider3D& a, Sphere* aData, Collider3D& b, Sphere* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;

    double ra = halfAx;
    double rb = halfBx;

    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;

    double dist2 = dx * dx + dy * dy + dz * dz;
    double r = ra + rb;
    if(dist2 > r * r) {
        return false;
    }
    double dist = sqrt(dist2);
    double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

    out.collisionNormalX = dx * invDist;
    out.collisionNormalY = dy * invDist;
    out.collisionNormalZ = dz * invDist;
    out.penetrationDepth = r - dist;
    return true;
}

bool collideSphere_AABB(Collider3D& a, Sphere* aData, Collider3D& b, AABB* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;

    double r = halfAx;

    double cx = max(bx - halfBx, min(ax, bx + halfBx));
    double cy = max(by - halfBy, min(ay, by + halfBy));
    double cz = max(bz - halfBz, min(az, bz + halfBz));

    double dx = ax - cx;
    double dy = ay - cy;
    double dz = az - cz;

    double dist2 = dx * dx + dy * dy + dz * dz;

    if (dist2 > r * r) {
        return false;
    }
    double dist = sqrt(dist2);
    double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

    out.collisionNormalX = dx * invDist;
    out.collisionNormalY = dy * invDist;
    out.collisionNormalZ = dz * invDist;
    out.penetrationDepth = r - dist;

    return true;
}

GLFWindow w("testing");

float cameraAngX = 0.0f;
float cameraAngY = 0.0f;

int main() {
    ecs.registerComponentType<mat4>();
    ResourceManager::addLocalResource("res/shaders");
    ResourceManager::addLocalResource("res/textures");

    renderSystem = new InstancedRenderSystem<mat4>(&ecs);
    scriptSystem = new ScriptSystem<MyScript>(&ecs);
    physicsSystem = new Physics3D(&ecs);
    scene = new Scene3D(&ecs);

    physicsSystem->registerCollision<AABB, AABB, collideAABB_AABB>();
    physicsSystem->registerCollision<Sphere, Sphere, collideSphere_Sphere>();
    physicsSystem->registerCollision<Sphere, AABB, collideSphere_AABB>();

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
    obj.mode = GL_TRIANGLES;

    Vertex3D vertices[]{
        {{ 1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},

        {{ 1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

        {{ 1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

        {{ 1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

        {{ 1.0f,  1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

        {{-1.0f,  1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}}
    };
    obj.uploadVertices(vertices, 36);

    uint32_t objectID = renderSystem->addObject(&obj);

    GLTexture gridTex = GLTexture::createTexture2D(2, 2);
    uint32_t pixels[]{
        0xFFAAAAAAu, 0xFF444444u,
        0xFF444444u, 0xFFAAAAAAu
    };
    gridTex.set2DTextureData(pixels, 2u, 2u, 0, 0, GL_RGBA, GL_UNSIGNED_INT);
    gridTex.setWrapMode(GL_REPEAT);
    RenderObject plane{shader, &gridTex};
    plane.mode = GL_TRIANGLE_STRIP;

    Vertex3D planeVerts[]{
        {{ 100.0f,  0.0f, -100.0f}, { 0.0f,  1.0f,  0.0f}, {  0.0f,   0.0f}},
        {{-100.0f,  0.0f, -100.0f}, { 0.0f,  1.0f,  0.0f}, {200.0f,   0.0f}},
        {{ 100.0f,  0.0f,  100.0f}, { 0.0f,  1.0f,  0.0f}, {  0.0f, 200.0f}},
        {{-100.0f,  0.0f,  100.0f}, { 0.0f,  1.0f,  0.0f}, {200.0f, 200.0f}}
    };
    plane.uploadVertices(planeVerts, 4);

    uint32_t planeID = renderSystem->addObject(&plane);

    Entity e2 = ecs.createEntity();
    e2.addComponent(diag(vec4(1.0)));
    e2.addComponent(RenderInstance<mat4>(planeID));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr, -100.0, -1.0, -100.0, 200.0, 2.0, 200.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr, -10.0, -1.0, -10.0, 20.0, 20.0, 2.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr, -10.0, -1.0, -10.0, 2.0, 20.0, 20.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr, -10.0, -1.0,   8.0, 20.0, 20.0, 2.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr,   8.0, -1.0, -10.0, 2.0, 20.0, 20.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<AABB>(nullptr, -10.0, 18.0, -10.0, 20.0, 2.0, 20.0, 0.0, 0.0));

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);

    //mat3 preRot = rotateX(0.01f) * rotateY(0.01f);
    //mat3 rot = rotateZ(0.01f);

    ecs.setup();

    float t = 0.0f;
    double dt = 1.0 / 165.0;
    for(uint32_t frame = 0u; w.isWindowOpen(); frame++) {
        uint64_t startTime = Time::nanoTime();

        if(frame % 10u == 0u) {
            for(int j = 0; j < 10; j++) {
                Entity e = ecs.createEntity();
                e.addComponent(diag(vec4(0.1)));
                e.addComponent(RenderInstance<mat4>(objectID));
                e.addComponent(PhysicsObject3D{0.0, 10.0, 0.2 * j, 0.001, 10.0, 0.0005 + 0.2 * j, 0.0, 0.0, 0.0});
                uint32_t id;
                ecs.getComponentID<PhysicsObject3D>(e, id);
                e.addComponent(DynamicCollider3D{{id}, physicsSystem->createCollider<Sphere>(nullptr, 0.0, 0.0, 0.0, 0.2, 0.2, 0.2, 0.0, 0.0), -0.1, -0.1, -0.1});
                e.addComponent(MyScript{});
            }
        }

        double px, py;
        w.getMousePos(px, py);
        float cx = w.getWidth() * 0.5f;
        float cy = w.getHeight() * 0.5f;
        float dx = (static_cast<float>(px) - cx) / cx;
        float dy = (static_cast<float>(py) - cy) / cy;
        cameraAngX += dy;
        cameraAngY -= dx;
        cam.cameraTransform = mat4(rotateX(cameraAngX) * rotateY(cameraAngY));
        cam.inverseCameraTransform = transpose(cam.cameraTransform);
        w.setMousePos(cx, cy);

        vec3 movement;

        if(Input::getKey(GLFW_KEY_W)) {
            movement -= vec3(cam.inverseCameraTransform[2]);
        }
        if(Input::getKey(GLFW_KEY_S)) {
            movement += vec3(cam.inverseCameraTransform[2]);
        }
        if(Input::getKey(GLFW_KEY_A)) {
            movement -= vec3(cam.inverseCameraTransform[0]);
        }
        if(Input::getKey(GLFW_KEY_D)) {
            movement += vec3(cam.inverseCameraTransform[0]);
        }
        if(dot(movement, movement) > 0.01f) {
            cam.cameraPos += 10.0f * static_cast<float>(dt) * normalize(movement);
        }

        ecs.update(dt);

        cameraBuffer.uploadPartialData(&cam, 1, 0);

        //std::cout << (Time::nanoTime() - startTime) << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render(obj);
        render(plane);
        w.update();
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
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
    glClearDepth(0.0);
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

    double dt = 1.0 / 165.0;
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
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
    }
}
#elif MODE == 2
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
