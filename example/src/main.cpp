#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <Engine.h>

#include <Window.h>

#include <BindingRegistry.h>
#include <Bloom.h>
#include <Camera3D.h>
#include <Camera3DSystem.h>
#include <Environment.h>
#include <FullscreenQuad.h>
#include <Light3DSystem.h>
#include <Material.h>
#include <RenderObject.h>
#include <RenderView3DSystem.h>
#include <Renderer.h>
#include <SSAO.h>
#include <ScriptSystem.h>
#include <ShadowSystem.h>
#include <Sky.h>
#include <Transform3D.h>
#include <World.h>
#include <ecs/ECS.h>
#include <input/Input.h>
#include <physics/2d/Physics2D.h>
#include <physics/3d/ColliderShape3D.h>
#include <physics/3d/Physics3D.h>
#include <prvl.h>
#include <rendering/GLFramebuffer.h>
#include <rendering/GLGBuffer.h>
#include <rendering/ShaderBuffer.h>
#include <scene/3d/Scene3D.h>
#include <serial/SceneSerializer.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>

#include <utils/perf.h>

#include "Scripts.h"


Window* window;


void registerComponentTypes() {
    ComponentRegistry::registerComponentType<PlayerScript>();
    // ...
}

struct Texture3DVertex {
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
};


// Rendering is still pretty manual, so you have to define what component is a instance,
// then define how you convert the component to instance data, in this case just a mat4
inline static void extractGlobal(const Transform3D* tx, mat4* i) {
    *i = tx->global;
}

// This will probably be automated once I make a proper model loader...
vec3 computeTangent(const vec3& a, const vec3& b, const vec3& c, const vec2& uva, const vec2& uvb, const vec2& uvc) {
    vec3 e1 = b - a;
    vec3 e2 = c - a;
    vec2 duv1 = uvb - uva;
    vec2 duv2 = uvc - uva;
    return (duv2.y * e1 - duv1.y * e2) / (duv1.x * duv2.y - duv2.x * duv1.y);
}

// Just a function which spawns a static block, with a mesh and a collider
void block(ECS& ecs, Scene3D* scene, Physics3D* physics, uint32_t parent, const vec3& pos, const vec3& size, uint32_t meshType) {
    Entity e = ecs.createEntity();
    mat4 tx = prvl::mat4(diag(size));
    tx[3u] = prvl::vec4(pos, 1.0f);
    e.addComponent(Transform3D{tx});
    e.addComponent(physics->createCollider<ColliderShape3D::AABB>(nullptr, pos.x, pos.y, pos.z, size.x, size.y, size.z, 1.0, 0.0));
    e.addComponent(RenderInstance{meshType});
    scene->addNode(parent, e);
}


// A shortcut to quickly exit the game
bool onInput(InputEvent* evt) {
    if (evt->type == InputEventType::KEY_EVENT && evt->keyEvent.pressed && evt->keyEvent.key == GLFW_KEY_F8) {
        window->close();
    }
    return false;
}


int main() {
    // Initialize the engine and register component types
    Engine::init();
    registerComponentTypes();


    // Create the window
    Window w("test", 500u, 500u, WindowMode::MAXIMIZED, false);
    window = &w;

    // Initialize the input system, currently only supports a single window
    Input::init(w);

    // Disable the mouse to stop the mouse from hitting the edges of the screen and allow for infinite travel
    Input::setMouseInputMode(w, MouseInputMode::DISABLED);

    // Create a environment buffer to store the global ambient color
    ShaderBuffer environmentBuffer(GL_DYNAMIC_DRAW);
    environmentBuffer.setSize(sizeof(Environment));
    ShaderManager::setValue("ENV_IDX", BindingRegistry::bindBufferBase(environmentBuffer, GL_UNIFORM_BUFFER));

    // Set the ambient color and upload it
    Environment env;
    env.ambientColor = prvl::vec3(0.2f, 0.5f, 1.0f);
    environmentBuffer.uploadPartialData(&env, 1, 0);

    // Initialize opengl state
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    // Create the world
    World world;
    ECS& ecs = world.ecs;


    // Set up all systems you'll use (some depend on others already being in the world)
    // The order in which the systems are added matters a tiny bit, sometimes you might get a off-by-one frame
    // delay caused by systems bing out of order.
    // This order is pretty standard though:

    // Physics engine
    Physics3D* physics = new Physics3D(&world, false);
    ColliderShape3D::registerBuiltinColliders(physics);
    world.addSystem(physics);

    // Scene graph
    Scene3D* scene = new Scene3D(&world);
    world.addSystem(scene);

    // Camera system
    world.addSystem(new Camera3DSystem(&ecs));

    // Render view system
    RenderView3DSystem* renderViewSystem = new RenderView3DSystem(&ecs);
    world.addSystem(renderViewSystem);

    // Rendering
    Renderer* renderer = new Renderer(&ecs);
    world.addSystem(renderer);

    ShadowSystem shadowSystem(5u, 5u);
    world.addSystem(new Light3DSystem(&ecs, &shadowSystem, 128u, 128u, 128u));

    // Scripting
    world.addSystem(new ScriptSystem<PlayerScript>(&world, false));

    // Set up ambient occlusion
    GLTexture aoTexture = GLTexture::createTexture2D(w.getWidth(), w.getHeight(), GL_R16F);
    ShaderManager::setValue("AO_TEXTURE_IDX", BindingRegistry::bindImageTexture(aoTexture, 0, false, 0, GL_READ_WRITE));

    // Set up SSAO
    SSAO ssao(w.getWidth(), w.getHeight());


    // Load a PBR material
    GLTexture albedo = ResourceManager::getResourceAsTexture("res/textures/BaseColor.jpg", -1);
    albedo.setInterpolation(true);
    albedo.setWrapMode(GL_REPEAT);
    albedo.setMipmapInterpolation(true);
    albedo.generateMipmap();
    albedo.setAnisotrophy(GLTexture::getMaxAnisotropy());

    GLTexture normal = ResourceManager::getResourceAsTexture("res/textures/Normal.jpg", -1);
    normal.setInterpolation(true);
    normal.setWrapMode(GL_REPEAT);
    normal.setMipmapInterpolation(true);
    normal.generateMipmap();
    normal.setAnisotrophy(GLTexture::getMaxAnisotropy());

    // For roughness and metallic, they have to be combined into one texture where red is roughness and green is metallic
    uint32_t wid, hei;
    uint32_t* roughness = ResourceManager::getResourceAsImage("res/textures/Roughness.jpg", wid, hei, true);
    uint32_t* metallic = ResourceManager::getResourceAsImage("res/textures/Metallic.jpg", wid, hei, true);
    uint32_t* roughnessMetallicData = alloc<uint32_t>(wid * hei);
    for (uint32_t i = 0u; i < wid * hei; i++) {
        roughnessMetallicData[i] = ((roughness[i] >> 8u) & 0xFFu) | ((metallic[i] >> 8u) & 0xFFu) << 8u;
    }

    GLTexture roughnessMetallic = GLTexture::createTexture2D(wid, hei, GL_RGBA8, -1);
    roughnessMetallic.set2DTextureData(roughnessMetallicData, wid, hei, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    roughnessMetallic.setInterpolation(true);
    roughnessMetallic.setWrapMode(GL_REPEAT);
    roughnessMetallic.setMipmapInterpolation(true);
    roughnessMetallic.generateMipmap();
    roughnessMetallic.setAnisotrophy(GLTexture::getMaxAnisotropy());

    // Create default PBR material
    Material material = Material::defaultTexture3D(albedo, normal, roughnessMetallic, prvl::vec3(0.05f));

    // Create a render group using the material and information on how to find and extract the instance data
    uint32_t groupID = renderer->createGroup<Transform3D, mat4, extractGlobal>(&material, ShaderManager::getVertexLayout(material.forward));

    // Create a cube mesh
    constexpr uint32_t vertexCount = 36u;
    Texture3DVertex verts[]{
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

        {{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

        {{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

        {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    };
    for (uint32_t i = 0u; i < vertexCount; i += 3u) {
        Texture3DVertex& a = verts[i];
        Texture3DVertex& b = verts[i + 1u];
        Texture3DVertex& c = verts[i + 2u];
        vec3 t = computeTangent(a.pos, b.pos, c.pos, a.uv, b.uv, c.uv);
        a.tangent = t;
        b.tangent = t;
        c.tangent = t;
        a.uv *= 5.0f;
        b.uv *= 5.0f;
        c.uv *= 5.0f;
    }

    // Create a render object using the group
    uint32_t cube = renderer->createObject(groupID);
    RenderObject& obj = renderer->getObject(cube);

    // Upload vertex data
    obj.uploadVertices(verts, vertexCount);


    // Create a scene

    // Creating a entity with only a transform as the root node
    Entity rootEntity = ecs.createEntity();
    rootEntity.addComponent(Transform3D{});

    uint32_t rootNode = scene->setRootNode(rootEntity);

    // Create a player entity with a PlayerScript component
    Entity player = ecs.createEntity();
    player.addComponent(Transform3D{});
    player.addComponent(PhysicsObject3D{0.0, 1.0, -5.0, 0.0, 1.0, -5.0, 0.0, 0.0, 0.0});
    player.addComponent(DynamicCollider3D{physics->createCollider<ColliderShape3D::Bean>(new ColliderShape3D::Bean{{0.5, 0.5, 0.5}, {0.5, 1.5, 0.5}, 0.5}, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.05, 0.0), -0.5, -1.0, -0.5, 1.0});
    uint32_t scriptID = player.addComponent(PlayerScript{});

    uint32_t playerNode = scene->addNode(rootNode, player);

    // The camera is attached to the player, so this entity is purely for that
    Entity cameraPivot = world.ecs.createEntity();
    cameraPivot.addComponent(Transform3D{});
    (*cameraPivot.getComponentPtr<Transform3D>()).local[3].y = 0.5f;

    uint32_t pivotNode = scene->addNode(playerNode, cameraPivot);

    // The entity which holds the actual camera
    Entity cameraHolder = world.ecs.createEntity();
    cameraHolder.addComponent(Transform3D{});
    uint32_t mainCamera = cameraHolder.addComponent(Camera3D{});

    scene->addNode(pivotNode, cameraHolder);

    // Create the sun
    Entity sun = ecs.createEntity();
    sun.addComponent(Transform3D{prvl::mat4(prvl::mat3(prvl::quat({1.0f, 0.0f, 0.0f}, -0.5f)))});
    sun.addComponent(DirectionalLight3D{prvl::vec3(1.0f, 0.7f, 0.5f), 5.0f});

    scene->addNode(playerNode, sun);

    // A entity can be used to simply group other entities together, which can be very handy sometimes
    Entity map = ecs.createEntity();
    map.addComponent(Transform3D{});

    uint32_t mapNode = scene->addNode(rootNode, map);

    // Build the "map"
    // Floor
    block(ecs, scene, physics, mapNode, {-10.0f, -1.0f, -10.0f}, {20.0f, 1.0f, 20.0f}, cube);

    // Doorway
    block(ecs, scene, physics, mapNode, {0.5f, 0.0f, -0.5f}, {1.0f, 2.0f, 1.0f}, cube);
    block(ecs, scene, physics, mapNode, {-1.5f, 0.0f, -0.5f}, {1.0f, 2.0f, 1.0f}, cube);
    block(ecs, scene, physics, mapNode, {-1.5f, 2.0f, -0.5f}, {3.0f, 1.0f, 1.0f}, cube);

    // Walls
    block(ecs, scene, physics, mapNode, {9.0f, 0.0f, -10.0f}, {1.0f, 10.0f, 20.0f}, cube);
    block(ecs, scene, physics, mapNode, {-10.0f, 0.0f, -10.0f}, {1.0f, 10.0f, 20.0f}, cube);
    block(ecs, scene, physics, mapNode, {-10.0f, 0.0f, 9.0f}, {20.0f, 10.0f, 1.0f}, cube);

    // Staircase
    for (uint32_t i = 0u; i <= 10u; i++) {
        block(ecs, scene, physics, mapNode, {-10.0f, static_cast<float>(i), -10.0f - i}, {20.0f, 1.0f, 1.0f}, cube);
    }

    // Upper platform
    block(ecs, scene, physics, mapNode, {10.0f, 10.0f, -20.0f}, {20.0f, 1.0f, 20.0f}, cube);

    // Lights
    for (uint32_t i = 0u; i < 10u; i++) {
        float t = static_cast<float>(i) / 10.0f;
        Entity light = ecs.createEntity();
        mat4 lightTx = prvl::mat4(prvl::mat3(prvl::quat({1.0f, 0.0f, 0.0f}, -0.5f)));
        lightTx[3] = prvl::vec4(0.0f, 10.0f, 10.0f, 1.0f);
        lightTx = prvl::mat4(prvl::mat3(prvl::quat({0.0f, 1.0f, 0.0f}, t * 6.28318530718f))) * lightTx;
        light.addComponent(Transform3D{lightTx});
        light.addComponent(SpotLight3D{prvl::vec3(t, 0.7f, 1.0f - t), 50.0f, 0.1f, 1.57079632679f * 0.5f, true, 0.1f});

        scene->addNode(mapNode, light);
    }

    // Here's where using a entity purely for making groups of entities comes in handy
    // This loads in a map scene which then is instansiated twice
    // The root node can then be translated and everything under it follows
    // Note that you'll need a scene file first, which is shown later
    /*
    SceneData data = ResourceManager::getResourceAsScene("res/main.scn");
    uint32_t mapANode = data.instantiate(&world, rootNode);
    uint32_t mapBNode = data.instantiate(&world, rootNode);

    Entity mapB = scene->getEntity(mapBNode);
    Transform3D* mapBTx = mapB.getComponentPtr<Transform3D>();
    mapBTx->local[3u] = prvl::vec4(50.0f, 0.0f, 0.0f, 1.0f);
    mapBTx->dirtyLocal = true;
    */

    // Create the main viewport
    Viewport viewport{prvl::uvec2(w.getWidth(), w.getHeight()), GL_RGB16F};

    // Making a view which will use the main camera to render onto the main viewport
    uint32_t mainView = renderViewSystem->createView(&viewport, mainCamera);

    // Set viewport reference in this particular player script
    ecs.getPtr<PlayerScript>(scriptID)->viewport = &viewport;

    // Create a sky background
    Sky sky;

    // Optionally load in a lens dirt texture
    // GLTexture lensDirt = ResourceManager::getResourceAsTexture("res/textures/LensDirt.jpg");
    // lensDirt.setInterpolation(true);
    GLTexture* lensDirtPtr = nullptr; // &lensDirt;

    // Create bloom with 9 mips
    Bloom bloom(w.getWidth(), w.getHeight(), 9u, GL_R11F_G11F_B10F);
    bloom.intensity = 100.0f;
    bloom.threshold = 1.5f;

    // Create a fullscreen quad with the final color processing pass including ACES tonemapping, gamma correction and dithering
    FullscreenQuad quad(ShaderManager::compileShaderFile("res/shaders/ColorProcess.frag", GL_FRAGMENT_SHADER));

    // Now that the scene is ready, we can call setup()
    // Now this will probably be subject to change as it currently only calls all setup() in all scripts
    // But as you might have scripts being added during runtime, this won't work
    ecs.setup();

    // Example of how you can serialize a subtree of the scene into a file
    // This can then be loaded using the scene load example
    /*
    SceneData data = SceneSerializer::serialize3D(&world, mapNode);
    ResourcePath res = "res/main.scn";
    std::ofstream file(res.getAbsolutePath(), std::ios::binary);
    file.write(reinterpret_cast<char*>(data.getData()), data.getSize());
    file.close();
    */

    // Register the event listener for input events
    world.eventBus.registerEventListener<InputEvent, onInput>();

    // Main update loop
    double dt = 1.0 / 165.0;
    for (uint32_t frame = 0u; w.isWindowOpen(); frame++) {
        uint64_t startTime = Time::nanoTime();

        // Poll input events
        Input::pollEvents(&world.eventBus);

        // Update the world
        world.update(dt);

        // Rendering
        // Shadows
        shadowSystem.renderShadows(renderViewSystem->cameraBinding, renderer);

        // Switch to main view
        renderViewSystem->use(mainView);
        {
            // Clear buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            // Depth prepass
            renderer->render(RenderMode::DEPTH);

            // Update ssao
            ssao.update(viewport.fbo.depth);

            // Resolve SSAO into main AO texture
            ssao.resolve(aoTexture, SSAOResolveMode::NO_BLUR);

            // Render sky background
            glDisable(GL_DEPTH_TEST);
            sky.render();

            // Main forward pass
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            renderer->render(RenderMode::FORWARD);
            glDepthFunc(GL_GREATER);
        }

        glDisable(GL_DEPTH_TEST);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Apply bloom
        bloom.render(viewport.fbo.color, lensDirtPtr);

        GLFramebuffer::unbind();
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        // Final color processing
        quad.setTexture(&viewport.fbo.color);
        quad.render();

        // Swap buffers
        w.update();

        // Update dt
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
    }
    Input::exit(w);
    w.exit();
    Engine::exit();
}