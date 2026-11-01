#ifndef PHYSICS3D_H_INCLUDED
#define PHYSICS3D_H_INCLUDED

#pragma once

#include <cstdint>
#include <immintrin.h>

#include <thread>

#include <structure/DynamicBVH.h>

#include <structure/DynamicArray.h>

#include <utils/perf.h>

#include <physics/3d/Collider3D.h>
#include <physics/3d/ColliderShape3DMetadata.h>
#include <physics/3d/CollisionInfo3D.h>
#include <physics/3d/DynamicCollider3D.h>
#include <physics/3d/PhysicsObject3D.h>
#include <physics/3d/PhysicsRayHit3D.h>
#include <physics/3d/RayHit3D.h>

#include <Transform3D.h>
#include <World.h>
#include <scene/3d/Scene3DNodeUpdatedEvent.h>

#include <Profiler.h>

using CollisionFunc3D = bool (*)(const void*, const void*, const Collider3D&, const Collider3D&, CollisionInfo3D&);
using RaycastFunction3D = float (*)(const void*, const float*, const float*, const float*, const float*, RayHit3D&);

struct ColliderShape3DRegistry {
    inline static DynamicArray<ColliderShape3DMetadata> colliderMetadata;
    inline static DynamicArray<CollisionFunc3D> collisionRegistry;
    inline static DynamicArray<RayIntersectionFunction> rayIntersectionFunctions;
    inline static DynamicArray<RaycastFunction3D> raycastFunctions;

    constexpr static uint32_t makeKey(uint32_t a, uint32_t b) {
        return (((a + b) * (a + b + 1u)) >> 1u) + b;
    }

    template <typename A, typename B, bool (*Func)(const A*, const B*, const Collider3D&, const Collider3D&, CollisionInfo3D&)>
    inline static bool inverseCollision(const A* aData, const B* bData, const Collider3D& a, const Collider3D& b, CollisionInfo3D& infoOut) {
        if (!Func(reinterpret_cast<const A*>(bData), reinterpret_cast<const B*>(aData), b, a, infoOut)) {
            return false;
        }
        infoOut.collisionNormal = -infoOut.collisionNormal;
        return true;
    }

    template <typename T>
    inline static void registerColliderShape() {
        ColliderShape3DMetadata metadata = ColliderShape3DMetadata::get<T>();
        while (metadata.typeId >= colliderMetadata.size()) {
            colliderMetadata.add(ColliderShape3DMetadata::invalid());
            rayIntersectionFunctions.add(nullptr);
            raycastFunctions.add(nullptr);
        }
        colliderMetadata[metadata.typeId] = metadata;
        float (*intersectFunc)(const T*, const float*, const float*, const float*, const float*) = &T::intersectRay;
        void (*castFunc)(const T*, const float*, const float*, const float*, const float*, RayHit3D&) = &T::raycast;
        rayIntersectionFunctions[metadata.typeId] = reinterpret_cast<RayIntersectionFunction>(intersectFunc);
        raycastFunctions[metadata.typeId] = reinterpret_cast<RaycastFunction3D>(castFunc);
    }

    template <typename A, typename B, bool (*Func)(const A*, const B*, const Collider3D&, const Collider3D&, CollisionInfo3D&)>
    inline static void registerCollisionFunction() {
        uint32_t idA = ColliderShape3DMetadata::typeOf<A>();
        uint32_t idB = ColliderShape3DMetadata::typeOf<B>();
        uint32_t keyAB = makeKey(idA, idB);
        uint32_t keyBA = makeKey(idB, idA);
        uint32_t maxKey = max(keyAB, keyBA);
        if (maxKey >= collisionRegistry.size()) {
            collisionRegistry.reserve(maxKey + 1u - collisionRegistry.size());
        }
        collisionRegistry[keyAB] = reinterpret_cast<CollisionFunc3D>(Func);
        if (idA != idB) {
            collisionRegistry[keyBA] = reinterpret_cast<CollisionFunc3D>(&ColliderShape3DRegistry::inverseCollision<A, B, Func>);
        }
    }
};

struct Physics3D {
private:
    DynamicArray<float> dynamicBounds;
    DynamicArray<float> staticBounds;

    DynamicBVH<3u> staticBVH;
    bool dirtyStatic = true;

    DynamicBVH<3u> dynamicBVH;

    double resolvingStrength = 0.5;

    ECS* ecs;

    struct Collision3D {
        DynamicCollider3D* a;
        DynamicCollider3D* b;
        Collider3D* bc;
        const dvec3 normal;
        const double penetrationDepth;

        Collision3D(DynamicCollider3D* a, DynamicCollider3D* b, CollisionInfo3D& info) : a(a), b(b), normal(info.collisionNormal), penetrationDepth(info.penetrationDepth) {
            this->bc = nullptr;
        }

        Collision3D(DynamicCollider3D* a, Collider3D* b, CollisionInfo3D& info) : a(a), bc(b), normal(info.collisionNormal), penetrationDepth(info.penetrationDepth) {
            this->b = nullptr;
        }
    };

    inline static uint32_t makeKey(uint32_t a, uint32_t b) {
        return (((a + b) * (a + b + 1)) >> 1u) + b;
    }

    bool collide(Collider3D& a, Collider3D& b, CollisionInfo3D& out) {
        uint32_t idA = a.typeId;
        uint32_t idB = b.typeId;
        uint32_t key = makeKey(idA, idB);
        if (key >= ColliderShape3DRegistry::collisionRegistry.size()) {
            return false;
        }
        CollisionFunc3D func = ColliderShape3DRegistry::collisionRegistry[key];
        if (!func) {
            return false;
        }
        return func(a.userData, b.userData, a, b, out);
    }

    void onStaticColliderAdded(Entity e, uint32_t id) {
        dirtyStatic = true;
    }

    void onStaticColliderRemoved(Entity e, uint32_t id) {
        dirtyStatic = true;
    }

    void onPhysicsObjectAdded(Entity e, uint32_t id) {
        Storage<PhysicsObject3D>& storage = ecs->storage<PhysicsObject3D>();
        uint32_t transformID;
        if (ecs->getComponentID<Transform3D>(e.entityID, transformID)) {
            uint32_t loc = storage.reg[id];
            /*Transform3D* tx = ecs->getPtr<Transform3D>(transformID);
            dvec3 pos = prvl::dvec3(prvl::vec3(tx->local[3u]));
            storage.objects.column<0>()[loc] = pos.x;
            storage.objects.column<1>()[loc] = pos.y;
            storage.objects.column<2>()[loc] = pos.z;
            storage.objects.column<3>()[loc] = pos.x;
            storage.objects.column<4>()[loc] = pos.y;
            storage.objects.column<5>()[loc] = pos.z;*/
            storage.objects.column<9>()[loc] = transformID;
        }
    }

    void onPhysicsObjectRemoved(Entity e, uint32_t id) {
    }

    void onDynamicColliderAdded(Entity e, uint32_t id) {
        uint32_t objID;
        if (ecs->getComponentID<PhysicsObject3D>(e.entityID, objID)) {
            ecs->getPtr<DynamicCollider3D>(id)->objectID = objID;
        }
    }

    void onDynamicColliderRemoved(Entity e, uint32_t id) {
    }

    bool onSceneNodeUpdated(Scene3DNodeUpdatedEvent* evt) {
        uint32_t entityID = evt->entityID;
        if (Collider3D* col = ecs->getComponentPtr<Collider3D>(entityID)) {
            Transform3D* tx = evt->tx;
            mat4& m = tx->global;
            vec3 pos = prvl::vec3(m[3u]);
            vec3 size = prvl::vec3(m[0u]);
            col->pos = prvl::dvec3(pos);
            col->size = prvl::dvec3(size);
            dirtyStatic = true;
        }
        return false;
    }

    void refreshStaticColliders() {
        VirtualArray<Collider3D>& staticColliders = ecs->storage<Collider3D>().data;
        if (staticColliders.size() == 0u) {
            return;
        }
        int idx = 0;
        staticBounds.ensureCapacity(staticColliders.size() * 6u);
        for (uint32_t i = 0u; i < staticColliders.size(); i++) {
            const Collider3D& col = staticColliders[i];
            staticBounds[idx++] = static_cast<float>(col.pos.x);
            staticBounds[idx++] = static_cast<float>(col.pos.y);
            staticBounds[idx++] = static_cast<float>(col.pos.z);
            staticBounds[idx++] = static_cast<float>(col.pos.x + col.size.x);
            staticBounds[idx++] = static_cast<float>(col.pos.y + col.size.y);
            staticBounds[idx++] = static_cast<float>(col.pos.z + col.size.z);
        }
        staticBVH.build(staticBounds.data(), staticColliders.size(),
            {
                .primitiveStride = sizeof(Collider3D),
                .typeOffset = offsetof(Collider3D, typeId),
                .dataOffset = offsetof(Collider3D, userData),
            });
    }

public:
    Physics3D(World* world, bool disabled) : ecs(&world->ecs) {
        ecs->registerComponentListener<Collider3D, Physics3D, &Physics3D::onStaticColliderAdded, &Physics3D::onStaticColliderRemoved>(this);
        ecs->registerComponentListener<PhysicsObject3D, Physics3D, &Physics3D::onPhysicsObjectAdded, &Physics3D::onPhysicsObjectRemoved>(this);
        ecs->registerComponentListener<DynamicCollider3D, Physics3D, &Physics3D::onDynamicColliderAdded, &Physics3D::onDynamicColliderRemoved>(this);
        world->eventBus.registerEventListener<Scene3DNodeUpdatedEvent, Physics3D, &Physics3D::onSceneNodeUpdated>(this);
        if (disabled) {
            ecs->registerUpdateCallback<Physics3D, &Physics3D::disabledPhysicsUpdate, UpdateOrder::PHYSICS>(this);
        } else {
            ecs->registerUpdateCallback<Physics3D, &Physics3D::physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
    }

    inline void markDirtyStatic() {
        dirtyStatic = true;
    }

    template <typename T>
    inline Collider3D createCollider(T* userData, double posX, double posY, double posZ, double sizeX, double sizeY, double sizeZ, double friction, double restitution) {
        return Collider3D{ColliderShape3DMetadata::typeOf<T>(), userData, posX, posY, posZ, sizeX, sizeY, sizeZ, friction, restitution};
    }

    inline PhysicsRayHit3D raycast(const vec3& rayPos, const vec3& rayDir) {
        PhysicsRayHit3D hit{};
        vec3 invDir = 1.0f / rayDir;
        Storage<Collider3D>& staticColliders = ecs->storage<Collider3D>();
        if (staticColliders.data.size() > 0u) {
            uint32_t hitIdx = staticBVH.queryIntersection(&rayPos[0u], &rayDir[0u], hit.distance, staticColliders.data.begin(), ColliderShape3DRegistry::rayIntersectionFunctions.data());
            if (hitIdx != UINT32_MAX) {
                RayHit3D pHit;
                Collider3D& col = staticColliders.data[hitIdx];
                ColliderShape3DRegistry::raycastFunctions[col.typeId](col.userData, &rayPos[0u], &rayDir[0u], &invDir[0u], staticBounds + hitIdx * 6u, pHit);
                hit.distance = pHit.distance;
                hit.depth = pHit.depth;
                hit.normal = pHit.normal;
                hit.didHit = pHit.didHit;
                hit.colliderID = staticColliders.reg.IDs[hitIdx];
                hit.entityID = staticColliders.entitySet[hitIdx];
                hit.isDynamic = false;
            }
        }
        Storage<DynamicCollider3D>& dynamicColliders = ecs->storage<DynamicCollider3D>();
        if (dynamicColliders.data.size() > 0u) {
            float dist = POSITIVE_INFINITY;
            uint32_t hitIdx = dynamicBVH.queryIntersection(&rayPos[0u], &rayDir[0u], dist, dynamicColliders.data.begin(), ColliderShape3DRegistry::rayIntersectionFunctions.data());
            if (hitIdx != UINT32_MAX && (!hit.didHit || dist < hit.distance)) {
                RayHit3D pHit;
                DynamicCollider3D& col = dynamicColliders.data[hitIdx];
                ColliderShape3DRegistry::raycastFunctions[col.impl.typeId](col.impl.userData, &rayPos[0u], &rayDir[0u], &invDir[0u], dynamicBounds + hitIdx * 6u, pHit);
                hit.distance = pHit.distance;
                hit.depth = pHit.depth;
                hit.normal = pHit.normal;
                hit.didHit = pHit.didHit;
                hit.colliderID = dynamicColliders.reg.IDs[hitIdx];
                hit.entityID = dynamicColliders.entitySet[hitIdx];
                hit.isDynamic = true;
            }
        }
        return hit;
    }

    void physicsUpdate(double dt) {
        PROFILE_SCOPE(Physics3D_Update)
        Storage<PhysicsObject3D>& storage = ecs->storage<PhysicsObject3D>();
        MultiDynamicArray<double, double, double, double, double, double, double, double, double, uint32_t>& objects = storage.objects;

        VirtualArray<Collider3D>& staticColliders = ecs->storage<Collider3D>().data;
        VirtualArray<DynamicCollider3D>& dynamicColliders = ecs->storage<DynamicCollider3D>().data;

        // uint64_t time[9];
        // time[0] = rdtsc();
        const double ddt = dt * dt;
        const __m256d vddt = _mm256_broadcast_sd(&ddt); // _mm256_set1_pd(ddt);
        const __m256d zero = _mm256_set1_pd(0.0);
        const __m256d two = _mm256_set1_pd(2.0);
        uint32_t updateI = 0u;
        for (; updateI + 3u < objects.size(); updateI += 4u) {
            double* srcPosX = objects.column<0>() + updateI;
            double* srcPrevPosX = objects.column<3>() + updateI;
            double* srcAccelerationX = objects.column<6>() + updateI;
            __m256d posX = _mm256_load_pd(srcPosX);
            __m256d prevPosX = _mm256_load_pd(srcPrevPosX);
            __m256d accX = _mm256_load_pd(srcAccelerationX);
            _mm256_store_pd(srcPosX, _mm256_fmadd_pd(accX, vddt, _mm256_fmsub_pd(posX, two, prevPosX)));
            _mm256_store_pd(srcPrevPosX, posX);
            _mm256_store_pd(srcAccelerationX, zero);

            double* srcPosY = objects.column<1>() + updateI;
            double* srcPrevPosY = objects.column<4>() + updateI;
            double* srcAccelerationY = objects.column<7>() + updateI;
            __m256d posY = _mm256_load_pd(srcPosY);
            __m256d prevPosY = _mm256_load_pd(srcPrevPosY);
            __m256d accY = _mm256_load_pd(srcAccelerationY);
            _mm256_store_pd(srcPosY, _mm256_fmadd_pd(accY, vddt, _mm256_fmsub_pd(posY, two, prevPosY)));
            _mm256_store_pd(srcPrevPosY, posY);
            _mm256_store_pd(srcAccelerationY, zero);

            double* srcPosZ = objects.column<2>() + updateI;
            double* srcPrevPosZ = objects.column<5>() + updateI;
            double* srcAccelerationZ = objects.column<8>() + updateI;
            __m256d posZ = _mm256_load_pd(srcPosZ);
            __m256d prevPosZ = _mm256_load_pd(srcPrevPosZ);
            __m256d accZ = _mm256_load_pd(srcAccelerationZ);
            _mm256_store_pd(srcPosZ, _mm256_fmadd_pd(accZ, vddt, _mm256_fmsub_pd(posZ, two, prevPosZ)));
            _mm256_store_pd(srcPrevPosZ, posZ);
            _mm256_store_pd(srcAccelerationZ, zero);
        }
        for (; updateI < objects.size(); updateI++) {
            double tx = objects.column<0>()[updateI];
            objects.column<0>()[updateI] += objects.column<0>()[updateI] - objects.column<3>()[updateI] + objects.column<6>()[updateI] * ddt;
            objects.column<3>()[updateI] = tx;
            objects.column<6>()[updateI] = 0.0;

            double ty = objects.column<1>()[updateI];
            objects.column<1>()[updateI] += objects.column<1>()[updateI] - objects.column<4>()[updateI] + objects.column<7>()[updateI] * ddt;
            objects.column<4>()[updateI] = ty;
            objects.column<7>()[updateI] = 0.0;

            double tz = objects.column<2>()[updateI];
            objects.column<2>()[updateI] += objects.column<2>()[updateI] - objects.column<5>()[updateI] + objects.column<8>()[updateI] * ddt;
            objects.column<5>()[updateI] = tz;
            objects.column<8>()[updateI] = 0.0;
        }
        // time[1] = rdtsc();
        if (dynamicColliders.size() == 0) {
            return;
        }
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider3D& col = dynamicColliders[i];
            const uint32_t loc = storage.reg[col.objectID];
            col.impl.pos = prvl::dvec3(objects.column<0>()[loc], objects.column<1>()[loc], objects.column<2>()[loc]) + col.offset;
        }
        // time[2] = rdtsc();
        if (dirtyStatic) {
            dirtyStatic = true;
            refreshStaticColliders();
        }
        uint32_t idx = 0u;
        dynamicBounds.ensureCapacity(dynamicColliders.size() * 6u);
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            const DynamicCollider3D& col = dynamicColliders[i];
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.x);
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.y);
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.z);
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.x + col.impl.size.x);
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.y + col.impl.size.y);
            dynamicBounds[idx++] = static_cast<float>(col.impl.pos.z + col.impl.size.z);
        }
        // time[3] = rdtsc();
        dynamicBVH.build(dynamicBounds.data(), dynamicColliders.size(),
            {
                .primitiveStride = sizeof(DynamicCollider3D),
                .typeOffset = offsetof(DynamicCollider3D, impl.typeId),
                .dataOffset = offsetof(DynamicCollider3D, impl.userData),
            });
        // time[4] = rdtsc();

        const uint32_t N = dynamicColliders.size() < 8u ? 1u : 8u;
        std::vector<std::thread> threads;
        threads.reserve(N);
        std::vector<std::vector<Collision3D>> result(N);

        // time[5] = rdtsc();

        if (N == 1u) {
            std::vector<Collision3D>& res = result[0u];
            alignas(16u) float query[6u];
            alignas(32u) uint32_t hits[32u];
            CollisionInfo3D info;
            for (uint32_t aIdx = 0u; aIdx < dynamicColliders.size(); aIdx++) {
                DynamicCollider3D& a = dynamicColliders[aIdx];
                //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                query[0u] = static_cast<float>(a.impl.pos.x);
                query[1u] = static_cast<float>(a.impl.pos.y);
                query[2u] = static_cast<float>(a.impl.pos.z);
                query[3u] = static_cast<float>(a.impl.pos.x + a.impl.size.x);
                query[4u] = static_cast<float>(a.impl.pos.y + a.impl.size.y);
                query[5u] = static_cast<float>(a.impl.pos.z + a.impl.size.z);
                uint32_t count = dynamicBVH.query(query, hits, 32u);
                if (count != 0u) {
                    for (uint32_t j = 0u; j < count; j++) {
                        uint32_t bIdx = hits[j];
                        if (aIdx >= bIdx) {
                            continue;
                        }
                        DynamicCollider3D& b = dynamicColliders[bIdx];
                        if (!collide(a.impl, b.impl, info)) {
                            continue;
                        }
                        res.emplace_back(&a, &b, info);
                    }
                }
                if (staticColliders.size() > 0u) {
                    count = staticBVH.query(query, hits, 32u);
                    if (count != 0u) {
                        for (uint32_t j = 0u; j < count; j++) {
                            uint32_t bIdx = hits[j];
                            Collider3D& b = staticColliders[bIdx];
                            if (!collide(a.impl, b, info)) {
                                continue;
                            }
                            res.emplace_back(&a, &b, info);
                        }
                    }
                }
            }
        } else {
            for (uint32_t i = 0u; i < N; i++) {
                uint32_t start = i * dynamicColliders.size() / N;
                uint32_t end = (i + 1u) * dynamicColliders.size() / N;
                threads.emplace_back([&, i, start, end]() {
                    std::vector<Collision3D>& res = result[i];
                    uint32_t* const stack = alloc<uint32_t>(dynamicColliders.size() * 2u);
                    alignas(16u) float query[6u];
                    alignas(32u) uint32_t hits[32u];
                    CollisionInfo3D info;
                    for (uint32_t aIdx = start; aIdx < end; aIdx++) {
                        DynamicCollider3D& a = dynamicColliders[aIdx];
                        //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                        query[0u] = static_cast<float>(a.impl.pos.x);
                        query[1u] = static_cast<float>(a.impl.pos.y);
                        query[2u] = static_cast<float>(a.impl.pos.z);
                        query[3u] = static_cast<float>(a.impl.pos.x + a.impl.size.x);
                        query[4u] = static_cast<float>(a.impl.pos.y + a.impl.size.y);
                        query[5u] = static_cast<float>(a.impl.pos.z + a.impl.size.z);
                        uint32_t count = dynamicBVH.query(query, hits, 32u, stack);
                        if (count != 0u) {
                            for (uint32_t j = 0u; j < count; j++) {
                                uint32_t bIdx = hits[j];
                                if (aIdx >= bIdx) {
                                    continue;
                                }
                                DynamicCollider3D& b = dynamicColliders[bIdx];
                                if (!collide(a.impl, b.impl, info)) {
                                    continue;
                                }
                                res.emplace_back(&a, &b, info);
                            }
                        }
                        if (staticColliders.size() > 0u) {
                            count = staticBVH.query(query, hits, 32u, stack);
                            if (count != 0u) {
                                for (uint32_t j = 0u; j < count; j++) {
                                    uint32_t bIdx = hits[j];
                                    Collider3D& b = staticColliders[bIdx];
                                    if (!collide(a.impl, b, info)) {
                                        continue;
                                    }
                                    res.emplace_back(&a, &b, info);
                                }
                            }
                        }
                    }
                    free(stack);
                });
            }
            for (std::thread& t : threads) {
                t.join();
            }
        }
        // time[6] = rdtsc();
        for (std::vector<Collision3D>& res : result) {
            for (Collision3D& col : res) {
                DynamicCollider3D* a = col.a;
                uint32_t objA = storage.reg[a->objectID];
                double& posAX = objects.column<0>()[objA];
                double& posAY = objects.column<1>()[objA];
                double& posAZ = objects.column<2>()[objA];
                double& prevPosAX = objects.column<3>()[objA];
                double& prevPosAY = objects.column<4>()[objA];
                double& prevPosAZ = objects.column<5>()[objA];

                DynamicCollider3D* b = col.b;

                const dvec3 normal = col.normal;
                double penetrationDepth = col.penetrationDepth * resolvingStrength;
                dvec3 d = normal * penetrationDepth;
                if (b) {
                    double mul = b->mass / (a->mass + b->mass);
                    posAX += d.x * mul;
                    posAY += d.y * mul;
                    posAZ += d.z * mul;
                    uint32_t objB = storage.reg[b->objectID];
                    mul = 1.0 - mul;
                    objects.column<0>()[objB] -= d.x * mul;
                    objects.column<1>()[objB] -= d.y * mul;
                    objects.column<2>()[objB] -= d.z * mul;
                } else {
                    double vx = posAX - prevPosAX;
                    double vy = posAY - prevPosAY;
                    double vz = posAZ - prevPosAZ;
                    posAX += d.x;
                    posAY += d.y;
                    posAZ += d.z;
                    double height = -(vx * normal.x + vy * normal.y + vz * normal.z);
                    if (height < 0.0) {
                        continue;
                    }
                    dvec3 nv = normal * height;
                    Collider3D& ac = a->impl;
                    double friction = 1.0 - ac.friction;
                    double restitution = ac.restitution;
                    double rx = (vx + nv.x) * friction + nv.x * restitution;
                    double ry = (vy + nv.y) * friction + nv.y * restitution;
                    double rz = (vz + nv.z) * friction + nv.z * restitution;
                    prevPosAX = posAX - rx;
                    prevPosAY = posAY - ry;
                    prevPosAZ = posAZ - rz;
                }
            }
        }
        // time[7] = rdtsc();
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider3D& col = dynamicColliders[i];
            uint32_t loc = storage.reg[col.objectID];
            double posX = objects.column<0>()[loc];
            double posY = objects.column<1>()[loc];
            double posZ = objects.column<2>()[loc];
            col.impl.pos = prvl::dvec3(posX, posY, posZ) + col.offset;
            if (Transform3D* tx = ecs->getPtr<Transform3D>(objects.column<9>()[loc])) {
                tx->position = prvl::vec3(posX, posY, posZ);
                tx->dirtyTRS = true;
            }
        }
        /*time[8] = rdtsc();
        uint32_t maxIdx = 0;
        uint64_t maxT = 0u;
        for(uint32_t i = 0u; i < 8u; i++) {
            uint64_t t = time[i + 1u] - time[i];
            if(t > maxT) {
                maxT = t;
                maxIdx = i;
            }
        }
        std::cout << maxIdx << " " << maxT << std::endl;*/
    }

    void disabledPhysicsUpdate(double dt) {
        Storage<PhysicsObject3D>& storage = ecs->storage<PhysicsObject3D>();
        MultiDynamicArray<double, double, double, double, double, double, double, double, double, uint32_t>& objects = storage.objects;

        VirtualArray<DynamicCollider3D>& dynamicColliders = ecs->storage<DynamicCollider3D>().data;

        const __m256d zero = _mm256_set1_pd(0.0);
        uint32_t updateI = 0u;
        for (; updateI + 3u < objects.size(); updateI += 4u) {
            double* srcPosX = objects.column<0>() + updateI;
            double* srcPrevPosX = objects.column<3>() + updateI;
            double* srcAccelerationX = objects.column<6>() + updateI;
            _mm256_store_pd(srcPrevPosX, _mm256_load_pd(srcPosX));
            _mm256_store_pd(srcAccelerationX, zero);

            double* srcPosY = objects.column<1>() + updateI;
            double* srcPrevPosY = objects.column<4>() + updateI;
            double* srcAccelerationY = objects.column<7>() + updateI;
            _mm256_store_pd(srcPrevPosY, _mm256_load_pd(srcPosY));
            _mm256_store_pd(srcAccelerationY, zero);

            double* srcPosZ = objects.column<2>() + updateI;
            double* srcPrevPosZ = objects.column<5>() + updateI;
            double* srcAccelerationZ = objects.column<8>() + updateI;
            _mm256_store_pd(srcPrevPosZ, _mm256_load_pd(srcPosZ));
            _mm256_store_pd(srcAccelerationZ, zero);
        }
        for (; updateI < objects.size(); updateI++) {
            objects.column<3>()[updateI] = objects.column<0>()[updateI];
            objects.column<6>()[updateI] = 0.0;

            objects.column<4>()[updateI] = objects.column<1>()[updateI];
            objects.column<7>()[updateI] = 0.0;

            objects.column<5>()[updateI] = objects.column<2>()[updateI];
            objects.column<8>()[updateI] = 0.0;
        }
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider3D& col = dynamicColliders[i];
            uint32_t loc = storage.reg[col.objectID];
            double posX = objects.column<0>()[loc];
            double posY = objects.column<1>()[loc];
            double posZ = objects.column<2>()[loc];
            col.impl.pos = prvl::dvec3(posX, posY, posZ) + col.offset;
            if (Transform3D* tx = ecs->getPtr<Transform3D>(objects.column<9>()[loc])) {
                tx->position = prvl::vec3(posX, posY, posZ);
                tx->dirtyTRS = true;
            }
        }
    }
};

template <>
struct Serial<Collider3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Collider3D* c = world->ecs.getPtr<Collider3D>(componentID);
        output.write(c->typeId);
        if (c->userData) {
            output.write<uint8_t>(1u);
            const ColliderShape3DMetadata& metadata = ColliderShape3DRegistry::colliderMetadata[c->typeId];
            output.write(c->userData, metadata.size);
        } else {
            output.write<uint8_t>(0u);
        }
        output.write(&c->pos, 8u * sizeof(double));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        Collider3D c;
        input.read(c.typeId);
        if (input.read<uint8_t>()) {
            const ColliderShape3DMetadata& metadata = ColliderShape3DRegistry::colliderMetadata[c.typeId];
            c.userData = _mm_malloc(metadata.size, metadata.alignment);
            input.read(c.userData, metadata.size);
        }
        input.read(&c.pos, 8u * sizeof(double));
        e.addComponent(c);
    }
};

template <>
struct Serial<DynamicCollider3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        DynamicCollider3D* c = world->ecs.getPtr<DynamicCollider3D>(componentID);
        output.write(c->impl.typeId);
        if (c->impl.userData) {
            output.write<uint8_t>(1u);
            const ColliderShape3DMetadata& metadata = ColliderShape3DRegistry::colliderMetadata[c->impl.typeId];
            output.write(c->impl.userData, metadata.size);
        } else {
            output.write<uint8_t>(0u);
        }
        output.write(&c->impl.size, 9u * sizeof(double));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        DynamicCollider3D c;
        input.read(c.impl.typeId);
        if (input.read<uint8_t>()) {
            const ColliderShape3DMetadata& metadata = ColliderShape3DRegistry::colliderMetadata[c.impl.typeId];
            c.impl.userData = _mm_malloc(metadata.size, metadata.alignment);
            input.read(c.impl.userData, metadata.size);
        }
        input.read(&c.impl.size, 9u * sizeof(double));
        c.objectID = UINT32_MAX;
        e.addComponent(c);
    }
};

template <>
struct Serial<PhysicsObject3D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        output.write(world->ecs.read<PhysicsObject3D>(componentID));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(input.read<PhysicsObject3D>());
    }
};

#endif
