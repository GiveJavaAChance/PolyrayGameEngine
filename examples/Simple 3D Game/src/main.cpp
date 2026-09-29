#include <Engine.h>

#include <Window.h>

#include <World.h>

#include <AmbientOcclusion.h>
#include <Bloom.h>
#include <RenderView3DSystem.h>
#include <Renderer.h>
#include <SSAO.h>
#include <SkinSystem.h>
#include <Sky.h>
#include <animation/AnimationSystem.h>
#include <gpu_types/GpuPBRMaterial.h>
#include <light/3d/Light3DSystem.h>
#include <physics/3d/ColliderShape3D.h>
#include <physics/3d/Physics3D.h>
#include <utils/Shape.h>

#include <Environment.h>

#include "Scripts.h"
#include "ShadowSystem.h"
#include "scene/3d/Scene3D.h"

Window* window;

void registerComponentTypes() {
    ComponentRegistry::registerComponentType<PlayerScript>();
    // ...
}

// Just a function which spawns a static block, with a mesh and a collider
void block(ECS& ecs, Scene3D* scene, Physics3D* physics, uint32_t parent, const vec3& pos, const vec3& size, uint32_t meshType) {
    Entity e = ecs.createEntity();
    e.addComponent(Transform3D{pos, prvl::quat(), size});
    e.addComponent(physics->createCollider<ColliderShape3D::AABB>(nullptr, pos.x, pos.y, pos.z, size.x, size.y, size.z, 1.0, 0.0));
    e.addComponent(RenderInstance{meshType});
    scene->addNode(parent, e, "Block");
}

// A shortcut to quickly exit the game when pressing F8
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

    uvec2 windowSize = prvl::uvec2(w.getWidth(), w.getHeight());

    // Initialize window input event listening (mouse and keyboard input)
    Input::initWindowInput(&w);

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

    // Initialize opengl state (subject to change later)
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glClearDepth(0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    // Create the world
    World world;
    ECS& ecs = world.ecs;

    ////////////////////
    // Create systems //
    ////////////////////

    // Set up all systems you'll use (some depend on others already being in the world)
    // The order in which the systems are added matters a tiny bit, sometimes you might get a off-by-one frame
    // delay caused by systems bing out of order.
    // This order is pretty standard though:

    // Physics engine
    world.createSystem<Physics3D>(&world, false);
    ColliderShape3D::registerBuiltinColliders(world.getSystem<Physics3D>());

    // Animation system (not used)
    // world.createSystem<AnimationSystem>(&world);

    // Scene graph system
    world.createSystem<Scene3D>(&world);

    // Camera system
    world.createSystem<Camera3DSystem>(&ecs);

    // Render view system
    world.createSystem<RenderView3DSystem>(&ecs);

    // Skinning system (not used)
    // world.createSystem<SkinSystem>(&ecs);

    // Renderer
    world.createSystem<Renderer>(&ecs);

    // Shadow and lights
    world.createSystem<ShadowSystem>(16u, 16u);
    world.createSystem<Light3DSystem>(&ecs, world.getSystem<ShadowSystem>(), 128u, 128u, 128u);

    // Scripting
    world.createSystem<ScriptSystem<PlayerScript>>(&world, false);

    // Keeping pointers to the systems for easier access
    Physics3D* physics = world.getSystem<Physics3D>();
    Scene3D* scene = world.getSystem<Scene3D>();
    RenderView3DSystem* renderViewSystem = world.getSystem<RenderView3DSystem>();
    Renderer* renderer = world.getSystem<Renderer>();
    ShadowSystem* shadowSystem = world.getSystem<ShadowSystem>();

    /////////////////////
    // Setup rendering //
    /////////////////////

    // Create the main viewport (with 4x MSAA)
    Viewport viewport{windowSize, GL_R11F_G11F_B10F, GL_DEPTH_COMPONENT32, 4u};

    // Create framebuffer for resolving MSAA
    GLFramebuffer resolvedFbo(windowSize.x, windowSize.y, GL_RGB16F);

    // Set up ambient occlusion
    AmbientOcclusion ambientOcclusion(windowSize);

    // Set up SSAO
    SSAO ssao(windowSize);

    // Create a sky background
    Sky sky;

    // Create bloom
    Bloom bloom(windowSize, GL_R11F_G11F_B10F);
    bloom.intensity = 100.0f;
    bloom.threshold = 1.5f;

    // Optionally load in a lens dirt texture (texture doesn't exist)
    // GLTexture lensDirt = ResourceManager::getResourceAsTexture("res/textures/LensDirt.jpg");
    // lensDirt.setInterpolation(true);
    GLTexture* lensDirtPtr = nullptr; // &lensDirt;

    // Create a fullscreen quad with the final color processing pass which includes ACES tonemapping, gamma correction and dithering
    FullscreenQuad colorProcess(ShaderManager::compileShaderFile("res/shaders/ColorProcess.frag", GL_FRAGMENT_SHADER));

    ////////////////////////////////////
    // Creating a material and a mesh //
    ////////////////////////////////////

    // Create default PBR material type
    MaterialType materialType = MaterialType::staticPBR();

    // What follows is mostly automated in the GLTF loader, this just shows how you'd do it manually

    // This is what tells the renderer what will supply the instance data to the material
    // In this case, we want to instance based on the global transform of the entities
    RenderGroupInfo groupInfo = RenderGroupInfo::create<Transform3D, mat4, &Transform3D::global>(materialType);

    // Create a render group using the group info
    uint32_t groupID = renderer->getOrCreateGroup(groupInfo);

    // Now we create the material instance which is what contains the material properties such as textures, color, etc.
    // Note: Types with a Gpu* prefix have the same size and layout as the GPU type
    GpuPBRMaterial material{};

    // The base color is multiplied by the base color texture, so we just use vec4(1.0) as the base color
    material.baseColor = prvl::vec4(1.0f);

    // Load base color texture
    GLTexture baseColorTexture = ResourceManager::getResourceAsTexture("res/textures/BaseColor.jpg", -1);
    baseColorTexture.setInterpolation(true);
    baseColorTexture.setWrapMode(GL_REPEAT);
    baseColorTexture.setMipmapInterpolation(true);
    baseColorTexture.generateMipmap();
    baseColorTexture.setAnisotrophy(GLTexture::getMaxAnisotropy());

    // Set the base color texture handle
    material.baseColorTexture = baseColorTexture.getBindlessHandle();

    // Load normal map texture
    GLTexture normalMapTexture = ResourceManager::getResourceAsTexture("res/textures/Normal.jpg", -1);
    normalMapTexture.setInterpolation(true);
    normalMapTexture.setWrapMode(GL_REPEAT);
    normalMapTexture.setMipmapInterpolation(true);
    normalMapTexture.generateMipmap();
    normalMapTexture.setAnisotrophy(GLTexture::getMaxAnisotropy());

    // Set the normal map texture handle
    material.normalMapTexture = normalMapTexture.getBindlessHandle();

    // Load metallic roughness texture
    // The textures for roughness and metallic are separated in this case, so we'll have to manually construct it
    // The convention used is the same as GLTF where blue is metallic and green is roughness
    uint32_t wid, hei;
    uint32_t* roughness = ResourceManager::getResourceAsImage("res/textures/Roughness.jpg", wid, hei, true);
    uint32_t* metallic = ResourceManager::getResourceAsImage("res/textures/Metallic.jpg", wid, hei, true);
    uint32_t* metallicRoughnessData = alloc<uint32_t>(wid * hei);
    for (uint32_t i = 0u; i < wid * hei; i++) {
        // A B G R
        metallicRoughnessData[i] = ((roughness[i] >> 8u) & 0xFFu) << 8u | ((metallic[i] >> 8u) & 0xFFu) << 16u;
    }

    GLTexture metallicRoughnessTexture = GLTexture::createTexture2D(wid, hei, GL_RGBA8, -1);
    metallicRoughnessTexture.set2DTextureData(metallicRoughnessData, wid, hei, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    free(metallicRoughnessData);

    metallicRoughnessTexture.setInterpolation(true);
    metallicRoughnessTexture.setWrapMode(GL_REPEAT);
    metallicRoughnessTexture.setMipmapInterpolation(true);
    metallicRoughnessTexture.generateMipmap();
    metallicRoughnessTexture.setAnisotrophy(GLTexture::getMaxAnisotropy());

    // Set the metallic roughness texture handle
    material.metallicRoughnessTexture = metallicRoughnessTexture.getBindlessHandle();

    // As with baseColor, we'll just use vec2(1.0) as the base metallic roughness
    material.baseMetallicRougness = prvl::vec2(1.0f);

    // The base color texture used is fully opaque, so no alpha cutoff needed
    material.alphaCutoff = 0.0f;

    // The material is not double sided
    material.doubleSided = false;

    // For the base reflectivity, we'll just use a simple platic-ish reflectivity
    material.F0 = prvl::vec3(0.05f);
    // A table of different values of F0: (https://learnopengl.com/PBR/Theory)
    // Material	                        F0
    // Water	                (0.02, 0.02, 0.02)
    // Plastic / Glass (Low)	(0.03, 0.03, 0.03)
    // Plastic High	            (0.05, 0.05, 0.05)
    // Glass (high) / Ruby	    (0.08, 0.08, 0.08)
    // Diamond	                (0.17, 0.17, 0.17)
    // Iron	                    (0.56, 0.57, 0.58)
    // Copper	                (0.95, 0.64, 0.54)
    // Gold	                    (1.00, 0.71, 0.29)
    // Aluminium	            (0.91, 0.92, 0.92)
    // Silver	                (0.95, 0.93, 0.88)

    // Create material instance in the group
    // Note: The RenderState is what pipeline state will be set to when rendering using this material,
    //       since this material isn't double sided, it's disabled here as well
    uint32_t materialID = renderer->addMaterialInstance(groupID, material, RenderState{.doubleSided = false});

    // Create a render object using the group and material instance
    uint32_t cube = renderer->createObject(groupID, materialID);

    // Get the render object
    RenderObject& obj = renderer->getObject(cube);

    // Define the vertex struct
    struct Vertex {
        vec3 position;
        vec3 normal;
        vec3 tangent;
        vec2 uv;
    };

    // The Shape util looks at what members the vertex type has (by name) and constructs the mesh with those attributes
    Mesh<Vertex> mesh = Shape::box<Vertex>(prvl::vec3(0.0f), prvl::vec3(1.0f));

    // Upload the mesh data
    obj.uploadMesh(mesh);

    //////////////////////
    // Creating a scene //
    //////////////////////

    // Creating a entity with only a transform as the root node
    Entity rootEntity = ecs.createEntity();
    rootEntity.addComponent(Transform3D{});

    uint32_t rootNode = scene->setRootNode(rootEntity, "Root");

    // Create a player entity with a PlayerScript component
    Entity player = ecs.createEntity();
    player.addComponent(Transform3D{});
    player.addComponent(PhysicsObject3D{0.0, 1.0, -5.0, 0.0, 1.0, -5.0, 0.0, 0.0, 0.0});
    player.addComponent(DynamicCollider3D{physics->createCollider<ColliderShape3D::Bean>(new ColliderShape3D::Bean{{0.5, 0.5, 0.5}, {0.5, 1.5, 0.5}, 0.5}, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.05, 0.0), -0.5, -1.0, -0.5, 1.0});
    uint32_t scriptID = player.addComponent(PlayerScript{});

    // Set viewport reference in this particular player script
    ecs.getPtr<PlayerScript>(scriptID)->viewport = &viewport;

    uint32_t playerNode = scene->addNode(rootNode, player, "Player");

    // The camera is attached to the player, so this entity is purely for that
    Entity cameraPivot = world.ecs.createEntity();
    cameraPivot.addComponent(Transform3D{});
    (*cameraPivot.getComponentPtr<Transform3D>()).local[3].y = 0.5f;

    uint32_t pivotNode = scene->addNode(playerNode, cameraPivot, "Camera Pivot");

    // The entity which holds the actual camera
    Entity cameraHolder = world.ecs.createEntity();
    cameraHolder.addComponent(Transform3D{});
    uint32_t mainCamera = cameraHolder.addComponent(Camera3D::perspective());

    scene->addNode(pivotNode, cameraHolder, "Camera Holder");

    // Create the sun
    Entity sun = ecs.createEntity();
    sun.addComponent(Transform3D{prvl::vec3(0.0f), prvl::quat({1.0f, 0.0f, 0.0f}, -0.5f)});
    sun.addComponent(DirectionalLight3D{prvl::vec3(1.0f, 0.7f, 0.5f), 5.0f});

    scene->addNode(playerNode, sun, "Sun");

    // A entity can be used to simply group other entities together, which can be very handy sometimes
    Entity map = ecs.createEntity();
    map.addComponent(Transform3D{});

    uint32_t mapNode = scene->addNode(rootNode, map, "Map");

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
        quat rot = prvl::quat({0.0f, 1.0f, 0.0f}, t * 6.28318530718f);
        vec3 lightPos = prvl::mat3(rot) * prvl::vec3(0.0f, 10.0f, 10.0f);
        light.addComponent(Transform3D{lightPos, rot * prvl::quat({1.0f, 0.0f, 0.0f}, -0.5f)});
        light.addComponent(SpotLight3D{prvl::vec3(t, 0.7f, 1.0f - t), 50.0f, 0.1f, 1.57079632679f * 0.5f});

        scene->addNode(mapNode, light, "Light");
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

    // Making a view which will use the main camera to render onto the main viewport
    uint32_t mainView = renderViewSystem->createView(&viewport, mainCamera);

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
    w.run([&](double dt) {
        // Poll input events
        Input::pollEvents(&world.eventBus);

        // Update the world
        world.update(dt);

        // Rendering

        // Shadows
        shadowSystem->renderShadows(renderViewSystem->cameraBinding, renderer);

        // If the main view is smaller than the viewport, we enable scissor test to avoid changing anything outside the view region
        glEnable(GL_SCISSOR_TEST);

        // Depth prepass
        renderViewSystem->use(mainView);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            renderer->render(RenderMode::DEPTH);
        }
        // Resolve MSAA depth first
        glBindFramebuffer(GL_READ_FRAMEBUFFER, viewport.fbo.ID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo.ID);
        glBlitFramebuffer(0, 0, windowSize.x, windowSize.y, 0, 0, windowSize.x, windowSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Update SSAO
        ssao.update(resolvedFbo.depth);

        // Resolve SSAO into main ambient occlusion texture
        ssao.resolve(SSAOResolveMode::NO_BLUR);

        // main forward color pass
        renderViewSystem->use(mainView);
        {
            // Render sky
            glDisable(GL_DEPTH_TEST);
            sky.render();

            // Render
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            renderer->render(RenderMode::FORWARD);
            glDepthFunc(GL_GREATER);
        }
        // Resolve MSAA color
        glBindFramebuffer(GL_READ_FRAMEBUFFER, viewport.fbo.ID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo.ID);
        glBlitFramebuffer(0, 0, windowSize.x, windowSize.y, 0, 0, windowSize.x, windowSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glDisable(GL_SCISSOR_TEST);

        // Start post processing
        resolvedFbo.bind();
        glViewport(0, 0, windowSize.x, windowSize.y);
        glDisable(GL_DEPTH_TEST);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Apply bloom
        bloom.render(resolvedFbo.color, lensDirtPtr);

        GLFramebuffer::unbind();
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        // Final color processing
        colorProcess.setTexture(&resolvedFbo.color);
        colorProcess.render();
    });

    // Exit
    Input::exit();
    Engine::exit();
}