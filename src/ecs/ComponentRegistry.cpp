#include <ecs/ECS.h>

#include <Camera2D.h>
#include <Camera3D.h>
#include <RenderInstance.h>
#include <SkinInstance.h>
#include <Transform2D.h>
#include <Transform3D.h>
#include <animation/AnimationSystem.h>
#include <light/3d/DirectionalLight3D.h>
#include <light/3d/PointLight3D.h>
#include <light/3d/SpotLight3D.h>
#include <physics/2d/Collider2D.h>
#include <physics/2d/DynamicCollider2D.h>
#include <physics/2d/Physics2D.h>
#include <physics/2d/PhysicsObject2D.h>
#include <physics/3d/Collider3D.h>
#include <physics/3d/DynamicCollider3D.h>
#include <physics/3d/Physics3D.h>
#include <physics/3d/PhysicsObject3D.h>

void ComponentRegistry::registerBuiltInComponents() {
    registerComponentType<Transform2D>();
    registerComponentType<Camera2D>();
    registerComponentType<PhysicsObject2D>();
    registerComponentType<Collider2D>();
    registerComponentType<DynamicCollider2D>();

    registerComponentType<Transform3D>();
    registerComponentType<Camera3D>();
    registerComponentType<PhysicsObject3D>();
    registerComponentType<Collider3D>();
    registerComponentType<DynamicCollider3D>();
    registerComponentType<DirectionalLight3D>();
    registerComponentType<SpotLight3D>();
    registerComponentType<PointLight3D>();

    registerComponentType<RenderInstance>();
    registerComponentType<SkinInstance>();

    registerComponentType<AnimationInstance>();
}