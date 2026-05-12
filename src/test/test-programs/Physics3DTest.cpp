#include <cstdint>
#include <iostream>

#include <base/glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLFWindow.h>
#include <prvl.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>
#include <rendering/ShaderBuffer.h>
#include <BindingRegistry.h>
#include <utils/perf.h>
#include <input/Input.h>
#include <RenderObject.h>
#include <ecs/ECS.h>

#include <physics/3d/Physics3D.h>
#include <InstancedRenderSystem.h>
#include <ScriptSystem.h>

#include <test/colliders.h>

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

struct Vertex3D {
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

struct DummyScript : Script {

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

struct PlayerScript : Script {

    void physicsUpdate(double dt) {
        PhysicsObject3D obj;
        if(getComponent<PhysicsObject3D>(obj)) {
            obj.accY = -3.0;
            vec3 movement = prvl::vec3();
            vec3 forward = -prvl::vec3(cam.inverseCameraTransform[2]);
            forward.y = 0.0f;
            forward = normalize(forward);
            if(Input::getKey(GLFW_KEY_W)) {
                movement += forward;
            }
            if(Input::getKey(GLFW_KEY_S)) {
                movement -= forward;
            }
            if(Input::getKey(GLFW_KEY_A)) {
                movement -= prvl::vec3(cam.inverseCameraTransform[0]);
            }
            if(Input::getKey(GLFW_KEY_D)) {
                movement += prvl::vec3(cam.inverseCameraTransform[0]);
            }
            if(dot(movement, movement) > 0.01f) {
                double mul = 2.0 * dt / length(movement);
                obj.prevPosX = obj.posX - movement.x * mul;
                obj.prevPosZ = obj.posZ - movement.z * mul;
            }
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
            cam.cameraPos = prvl::vec3(tr) + prvl::vec3(0.0f, 0.15f, 0.0f);
        }
    }
};

InstancedRenderSystem<mat4>* renderSystem;
ScriptSystem<DummyScript>* scriptSystem;
ScriptSystem<PlayerScript>* playerScriptSystem;
Physics3D* physicsSystem;

struct Gizmos {
    DynamicArray<vec3> gizmosData;
    RenderObject* obj;

    Gizmos(RenderObject* obj) : obj(obj) {
    }

    void setSize(int count) {
        gizmosData.ensureCapacity(count * 3);
    }

    void pushCube(vec3 pos, vec3 size, vec3 color) {
        gizmosData.add(pos);
        gizmosData.add(size);
        gizmosData.add(color);
    }

    void update(double dt) {
        obj->uploadInstances(gizmosData.data(), gizmosData.size());
        obj->instanceCount = gizmosData.size() / 3;
        gizmosData.clear();
    }
};

float cameraAngX = 0.0f;
float cameraAngY = 0.0f;

Gizmos* gizmos;
bool first = true;

int main() {
    ecs.registerComponentType<mat4>();
    ResourceManager::addLocalResource("res/shaders");
    ResourceManager::addLocalResource("res/textures");

    renderSystem = new InstancedRenderSystem<mat4>(&ecs);
    scriptSystem = new ScriptSystem<DummyScript>(&ecs);
    playerScriptSystem = new ScriptSystem<PlayerScript>(&ecs);
    physicsSystem = new Physics3D(&ecs);

    registerAllColliders(physicsSystem);

    w.createFrame(500, 500, false, true, false);

    cam.projection = reverseZPerspectiveProjection(1.57079632679f, static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight()), 0.1f);
    cam.inverseProjection = inverse(cam.projection);

    ShaderBuffer cameraBuffer(GL_DYNAMIC_DRAW);
    ShaderBuffer envBuffer(GL_DYNAMIC_DRAW);
    cameraBuffer.setSize(sizeof(Camera3D));
    envBuffer.setSize(sizeof(Environment));

    env.sunColor = prvl::vec3(1.0f, 1.0f, 0.5f);
    env.ambientColor = prvl::vec3(0.2f, 0.5f, 1.0f);
    env.sunDir = normalize(prvl::vec3(1.0f));

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
    //uint32_t pixel = 0xFFFFFFFFuu;
    //tex.set2DTextureData(&pixel, 1, 1, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    RenderObject obj{shader, &tex};
    obj.mode = GL_TRIANGLE_STRIP;

    /*Vertex3D vertices[]{
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
    obj.uploadVertices(vertices, 36);*/
    int detail = 32;
    Vertex3D vertices[detail * 2];
    for(int i = 0; i < detail; i++) {
        float t = static_cast<float>(i) / static_cast<float>(detail - 1);
        float a = t * 6.28318530718f;
        float s = sin(a);
        float c = cos(a);
        vertices[i * 2] = {{c, -1.0f, s}, {c, 0.0f, s}, {t, 0.0f}};
        vertices[i * 2 + 1] = {{c,  1.0f, s}, {c, 0.0f, s}, {t, 1.0f}};
    }
    obj.uploadVertices(vertices, detail * 2);

    uint32_t objectID = renderSystem->addObject(&obj);

    GLTexture gridTex = GLTexture::createTexture2D(2, 2);
    uint32_t pixels[]{
        0xFFAAAAAAu, 0xFF444444u,
        0xFF444444u, 0xFFAAAAAAu
    };
    gridTex.set2DTextureData(pixels, 2u, 2u, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    gridTex.setWrapMode(GL_REPEAT);
    RenderObject plane{shader, &gridTex};
    plane.mode = GL_TRIANGLE_STRIP;

    Vertex3D planeVerts[]{
        {{ 100.0f,  0.0f, -100.0f}, { 0.0f,  1.0f,  0.0f}, {  0.0f,   0.0f}},
        {{-100.0f,  0.0f, -100.0f}, { 0.0f,  1.0f,  0.0f}, {100.0f,   0.0f}},
        {{ 100.0f,  0.0f,  100.0f}, { 0.0f,  1.0f,  0.0f}, {  0.0f, 100.0f}},
        {{-100.0f,  0.0f,  100.0f}, { 0.0f,  1.0f,  0.0f}, {100.0f, 100.0f}}
    };
    plane.uploadVertices(planeVerts, 4);

    uint32_t planeID = renderSystem->addObject(&plane);

    Entity e2 = ecs.createEntity();
    e2.addComponent(diag(prvl::vec4(1.0)));
    e2.addComponent(RenderInstance<mat4>(planeID));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr, -100.0, -2.0, -100.0, 200.0, 2.0, 200.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr, -10.0, -2.0, -10.0, 20.0, 20.0, 2.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr, -10.0, -2.0, -10.0, 2.0, 20.0, 20.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr, -10.0, -2.0,   8.0, 20.0, 20.0, 2.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr,   8.0, -2.0, -10.0, 2.0, 20.0, 20.0, 0.0, 0.0));
    e2.addComponent(physicsSystem->createCollider<CollisionShape::AABB>(nullptr, -10.0, 18.0, -10.0, 20.0, 2.0, 20.0, 0.0, 0.0));

    GLuint cubeVert = ShaderManager::compileShaderFile("Cube3D.vert", GL_VERTEX_SHADER);
    GLuint cubeFrag = ShaderManager::compileShaderFile("Cube3D.frag", GL_FRAGMENT_SHADER);

    ShaderProgram cubeShader = ShaderManager::createProgram({cubeVert, cubeFrag});

    RenderObject cubeObj{cubeShader};
    cubeObj.mode = GL_LINES;

    vec3 cubeVerts[]{
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},

        {0.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},

        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f}
    };
    cubeObj.uploadVertices(cubeVerts, 24);
    //gizmos = new Gizmos(&cubeObj);

    /*physicsSystem->addDynamicQueryCallback([](BVH<3>& bvh) {
        if(first) {
            first = false;
            int count = bvh.getNodeCount();
            float* b = bvh.getNodeBounds();
            gizmos->setSize(count);
            for(int i = 0; i < count; i++) {
                vec3 min = vec3(b[i * 6 + 0], b[i * 6 + 1], b[i * 6 + 2]);
                vec3 max = vec3(b[i * 6 + 3], b[i * 6 + 4], b[i * 6 + 5]);
                gizmos->pushCube(min, max - min, vec3(1.0f, 0.0f, 0.0f));
            }
        }
    });*/

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);

    //mat3 preRot = rotateX(0.01f) * rotateY(0.01f);
    //mat3 rot = rotateZ(0.01f);

    CollisionShape::Bean* beanShape = new CollisionShape::Bean{{0.1, 0.1, 0.1}, {0.1, 0.3, 0.1}, 0.1};

    ecs.setup();

    w.hideCursor(true);

    for(uint32_t z = 0u; z < 10u; z++) {
        for(uint32_t y = 0u; y < 10u; y++) {
            for(uint32_t x = 0u; x < 10u; x++) {
                Entity e = ecs.createEntity();
                e.addComponent(diag(prvl::vec4(0.1f)));
                (*e.getComponentPtr<mat4>())[1].y = 0.2;
                e.addComponent(RenderInstance<mat4>(objectID));
                e.addComponent(PhysicsObject3D{0.3 * x, 0.2 + 0.4 * y, 0.3 * z, 0.3 * x, 0.2 + 0.4 * y, 0.3 * z, 0.0, 0.0, 0.0});
                uint32_t id;
                ecs.getComponentID<PhysicsObject3D>(e, id);
                e.addComponent(DynamicCollider3D{{id}, physicsSystem->createCollider<CollisionShape::Bean>(beanShape, 0.0, 0.0, 0.0, 0.2, 0.4, 0.2, 0.05, 0.0), -0.1, -0.2, -0.1});
                e.addComponent(DummyScript{});
            }
        }
    }

    Entity player = ecs.createEntity();
    player.addComponent(diag(prvl::vec4(0.1f)));
    (*player.getComponentPtr<mat4>())[1].y = 0.2;
    player.addComponent(PhysicsObject3D{-5.0, 0.5, -5.0, -5.0, 0.5, -5.0, 0.0, 0.0, 0.0});
    uint32_t id;
    ecs.getComponentID<PhysicsObject3D>(player, id);
    player.addComponent(DynamicCollider3D{{id}, physicsSystem->createCollider<CollisionShape::Bean>(beanShape, 0.0, 0.0, 0.0, 0.2, 0.4, 0.2, 0.05, 0.0), -0.1, -0.2, -0.1});
    player.addComponent(PlayerScript{});

    double dt = 1.0 / 165.0;
    for(uint32_t frame = 0u; w.isWindowOpen(); frame++) {
        uint64_t startTime = Time::nanoTime();

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

        first = true;
        ecs.update(dt);

        //gizmos->update(dt);

        cameraBuffer.uploadPartialData(&cam, 1, 0);

        //std::cout << (Time::nanoTime() - startTime) << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render(obj);
        render(plane);
        render(cubeObj);
        w.update();
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
    }
}
