#ifndef PHYSICS2D_H_INCLUDED
#define PHYSICS2D_H_INCLUDED

#pragma once

#include <cstdint>
#include <immintrin.h>

#include <iostream>

#include <thread>

#include <structure/DynamicBVH.h>

#include <structure/DynamicArray.h>

#include <utils/perf.h>

#include <physics/2d/Collider2D.h>
#include <physics/2d/ColliderShape2DMetadata.h>
#include <physics/2d/CollisionInfo2D.h>
#include <physics/2d/DynamicCollider2D.h>
#include <physics/2d/PhysicsObject2D.h>
#include <physics/2d/PhysicsRayHit2D.h>
#include <physics/2d/RayHit2D.h>

#include <Transform2D.h>
#include <World.h>
#include <scene/2d/Scene2DNodeUpdatedEvent.h>

#include <Profiler.h>

using CollisionFunc2D = bool (*)(const void*, const void*, const Collider2D&, const Collider2D&, CollisionInfo2D&);
using RaycastFunction2D = float (*)(const void*, const float*, const float*, const float*, const float*, RayHit2D&);

struct ColliderShape2DRegistry {
    inline static DynamicArray<ColliderShape2DMetadata> colliderMetadata;
    inline static DynamicArray<CollisionFunc2D> collisionRegistry;
    inline static DynamicArray<RayIntersectionFunction> rayIntersectionFunctions;
    inline static DynamicArray<RaycastFunction2D> raycastFunctions;

    constexpr static uint32_t makeKey(uint32_t a, uint32_t b) {
        return (((a + b) * (a + b + 1u)) >> 1u) + b;
    }

    template <typename A, typename B, bool (*Func)(const A*, const B*, const Collider2D&, const Collider2D&, CollisionInfo2D&)>
    inline static bool inverseCollision(const A* aData, const B* bData, const Collider2D& a, const Collider2D& b, CollisionInfo2D& infoOut) {
        if (!Func(reinterpret_cast<const A*>(bData), reinterpret_cast<const B*>(aData), b, a, infoOut)) {
            return false;
        }
        infoOut.collisionNormalX = -infoOut.collisionNormalX;
        infoOut.collisionNormalY = -infoOut.collisionNormalY;
        return true;
    }

    template <typename T>
    inline static void registerColliderShape() {
        ColliderShape2DMetadata metadata = ColliderShape2DMetadata::get<T>();
        while (metadata.typeId >= colliderMetadata.size()) {
            colliderMetadata.add(ColliderShape2DMetadata::invalid());
            rayIntersectionFunctions.add(nullptr);
            raycastFunctions.add(nullptr);
        }
        colliderMetadata[metadata.typeId] = metadata;
        float (*intersectFunc)(const T*, const float*, const float*, const float*, const float*) = &T::intersectRay;
        void (*castFunc)(const T*, const float*, const float*, const float*, const float*, RayHit2D&) = &T::raycast;
        rayIntersectionFunctions[metadata.typeId] = reinterpret_cast<RayIntersectionFunction>(intersectFunc);
        raycastFunctions[metadata.typeId] = reinterpret_cast<RaycastFunction2D>(castFunc);
    }

    template <typename A, typename B, bool (*Func)(const A*, const B*, const Collider2D&, const Collider2D&, CollisionInfo2D&)>
    inline static void registerCollisionFunction() {
        uint32_t idA = ColliderShape2DMetadata::typeOf<A>();
        uint32_t idB = ColliderShape2DMetadata::typeOf<B>();
        uint32_t keyAB = makeKey(idA, idB);
        uint32_t keyBA = makeKey(idB, idA);
        uint32_t maxKey = max(keyAB, keyBA);
        if (maxKey >= collisionRegistry.size()) {
            collisionRegistry.reserve(maxKey + 1u - collisionRegistry.size());
        }
        collisionRegistry[keyAB] = reinterpret_cast<CollisionFunc2D>(Func);
        if (idA != idB) {
            collisionRegistry[keyBA] = reinterpret_cast<CollisionFunc2D>(&ColliderShape2DRegistry::inverseCollision<A, B, Func>);
        }
    }
};

struct Physics2D {
private:
    DynamicArray<float> dynamicBounds;
    DynamicArray<float> staticBounds;

    DynamicBVH<2u> staticBVH;
    bool dirtyStatic = true;

    DynamicBVH<2u> dynamicBVH;

    double resolvingStrength = 0.5;

    ECS* ecs;

    struct Collision2D {
        DynamicCollider2D* a;
        DynamicCollider2D* b;
        Collider2D* bc;
        const double normalX, normalY;
        const double penetrationDepth;

        Collision2D(DynamicCollider2D* a, DynamicCollider2D* b, CollisionInfo2D& info) : a(a), b(b), normalX(info.collisionNormalX), normalY(info.collisionNormalY), penetrationDepth(info.penetrationDepth) {
            this->bc = nullptr;
        }

        Collision2D(DynamicCollider2D* a, Collider2D* b, CollisionInfo2D& info) : a(a), bc(b), normalX(info.collisionNormalX), normalY(info.collisionNormalY), penetrationDepth(info.penetrationDepth) {
            this->b = nullptr;
        }
    };

    inline static uint32_t makeKey(uint32_t a, uint32_t b) {
        return (((a + b) * (a + b + 1)) >> 1u) + b;
    }

    bool collide(const Collider2D& a, const Collider2D& b, CollisionInfo2D& out) {
        uint32_t idA = a.typeId;
        uint32_t idB = b.typeId;
        uint32_t key = makeKey(idA, idB);
        if (key >= ColliderShape2DRegistry::collisionRegistry.size()) {
            return false;
        }
        CollisionFunc2D func = ColliderShape2DRegistry::collisionRegistry[key];
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
        Storage<PhysicsObject2D>& storage = ecs->view<PhysicsObject2D>();
        uint32_t transformID;
        if (ecs->getComponentID<Transform2D>(e.entityID, transformID)) {
            uint32_t loc = storage.reg[id];
            /*Transform2D* tx = ecs->getPtr<Transform2D>(transformID);
            dvec2 pos = prvl::dvec2(prvl::vec2(tx->local[2u]));
            storage.objects.column<0>()[loc] = pos.x;
            storage.objects.column<1>()[loc] = pos.y;
            storage.objects.column<2>()[loc] = pos.x;
            storage.objects.column<3>()[loc] = pos.y;*/
            storage.objects.column<6>()[loc] = transformID;
        }
    }

    void onPhysicsObjectRemoved(Entity e, uint32_t id) {
    }

    void onDynamicColliderAdded(Entity e, uint32_t id) {
        uint32_t objID;
        if (ecs->getComponentID<PhysicsObject2D>(e.entityID, objID)) {
            ecs->getPtr<DynamicCollider2D>(id)->object.ID = objID;
        }
    }

    void onDynamicColliderRemoved(Entity e, uint32_t id) {
    }

    bool onSceneNodeUpdated(Scene2DNodeUpdatedEvent* evt) {
        uint32_t entityID = evt->entityID;
        if (Collider2D* col = ecs->getComponentPtr<Collider2D>(entityID)) {
            Transform2D* tx = evt->tx;
            mat3& m = tx->global;
            vec2 pos = prvl::vec2(m[2u]);
            col->posX = pos.x;
            col->posY = pos.y;
            col->sizeX = m[0u][0u];
            col->sizeY = m[1u][1u];
            dirtyStatic = true;
        }
        return false;
    }

    void refreshStaticColliders() {
        dirtyStatic = false;
        VirtualArray<Collider2D>& staticColliders = ecs->view<Collider2D>().data;
        if (staticColliders.size() == 0) {
            return;
        }
        int idx = 0;
        staticBounds.ensureCapacity(staticColliders.size() * 4u);
        for (uint32_t i = 0u; i < staticColliders.size(); i++) {
            const Collider2D& col = staticColliders[i];
            staticBounds[idx++] = static_cast<float>(col.posX);
            staticBounds[idx++] = static_cast<float>(col.posY);
            staticBounds[idx++] = static_cast<float>(col.posX + col.sizeX);
            staticBounds[idx++] = static_cast<float>(col.posY + col.sizeY);
        }
        staticBVH.build(staticBounds.data(), staticColliders.size(),
            {
                .primitiveStride = sizeof(Collider2D),
                .typeOffset = offsetof(Collider2D, typeId),
                .dataOffset = offsetof(Collider2D, userData),
            });
    }

public:
    Physics2D(World* world, bool disabled) : ecs(&world->ecs) {
        ecs->registerComponentListener<Collider2D, Physics2D, &Physics2D::onStaticColliderAdded, &Physics2D::onStaticColliderRemoved>(this);
        ecs->registerComponentListener<PhysicsObject2D, Physics2D, &Physics2D::onPhysicsObjectAdded, &Physics2D::onPhysicsObjectRemoved>(this);
        ecs->registerComponentListener<DynamicCollider2D, Physics2D, &Physics2D::onDynamicColliderAdded, &Physics2D::onDynamicColliderRemoved>(this);
        world->eventBus.registerEventListener<Scene2DNodeUpdatedEvent, Physics2D, &Physics2D::onSceneNodeUpdated>(this);
        if (disabled) {
            ecs->registerUpdateCallback<Physics2D, &Physics2D::disabledPhysicsUpdate, UpdateOrder::PHYSICS>(this);
        } else {
            ecs->registerUpdateCallback<Physics2D, &Physics2D::physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
    }

    inline void markDirtyStatic() {
        dirtyStatic = true;
    }

    template <typename T>
    inline Collider2D createCollider(T* userData, double posX, double posY, double sizeX, double sizeY, double friction, double restitution) {
        return Collider2D{ColliderShape2DMetadata::typeOf<T>(), userData, posX, posY, sizeX, sizeY, friction, restitution};
    }

    inline PhysicsRayHit2D raycast(const vec2& rayPos, const vec2& rayDir) {
        PhysicsRayHit2D hit{};
        vec2 invDir = 1.0f / rayDir;
        Storage<Collider2D>& staticColliders = ecs->view<Collider2D>();
        if (staticColliders.data.size() > 0u) {
            uint32_t hitIdx = staticBVH.queryIntersection(&rayPos[0u], &rayDir[0u], hit.distance, staticColliders.data.begin(), ColliderShape2DRegistry::rayIntersectionFunctions.data());
            if (hitIdx != UINT32_MAX) {
                RayHit2D pHit;
                Collider2D& col = staticColliders.data[hitIdx];
                ColliderShape2DRegistry::raycastFunctions[col.typeId](col.userData, &rayPos[0u], &rayDir[0u], &invDir[0u], staticBounds + hitIdx * 4u, pHit);
                hit.distance = pHit.distance;
                hit.depth = pHit.depth;
                hit.normal = pHit.normal;
                hit.didHit = pHit.didHit;
                hit.colliderID = staticColliders.reg.IDs[hitIdx];
                hit.entityID = staticColliders.entitySet[hitIdx];
                hit.isDynamic = false;
            }
        }
        Storage<DynamicCollider2D>& dynamicColliders = ecs->view<DynamicCollider2D>();
        if (dynamicColliders.data.size() > 0u) {
            float dist = POSITIVE_INFINITY;
            uint32_t hitIdx = dynamicBVH.queryIntersection(&rayPos[0u], &rayDir[0u], dist, dynamicColliders.data.begin(), ColliderShape2DRegistry::rayIntersectionFunctions.data());
            if (hitIdx != UINT32_MAX && (!hit.didHit || dist < hit.distance)) {
                RayHit2D pHit;
                DynamicCollider2D& col = dynamicColliders.data[hitIdx];
                ColliderShape2DRegistry::raycastFunctions[col.impl.typeId](col.impl.userData, &rayPos[0u], &rayDir[0u], &invDir[0u], staticBounds + hitIdx * 4u, pHit);
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
        PROFILE_SCOPE(Physics2D_Update)
        Storage<PhysicsObject2D>& storage = ecs->view<PhysicsObject2D>();
        MultiDynamicArray<double, double, double, double, double, double, uint32_t>& objects = storage.objects;

        VirtualArray<Collider2D>& staticColliders = ecs->view<Collider2D>().data;
        VirtualArray<DynamicCollider2D>& dynamicColliders = ecs->view<DynamicCollider2D>().data;

        // uint64_t time[9];
        // time[0] = rdtsc();
        const double ddt = dt * dt;
        const __m256d vddt = _mm256_broadcast_sd(&ddt); // _mm256_set1_pd(ddt);
        const __m256d zero = _mm256_set1_pd(0.0);
        const __m256d two = _mm256_set1_pd(2.0);
        uint32_t updateI = 0u;
        for (; updateI + 3u < objects.size(); updateI += 4u) {
            double* srcPosX = objects.column<0>() + updateI;
            double* srcPrevPosX = objects.column<2>() + updateI;
            double* srcAccelerationX = objects.column<4>() + updateI;
            __m256d posX = _mm256_load_pd(srcPosX);
            __m256d prevPosX = _mm256_load_pd(srcPrevPosX);
            __m256d accX = _mm256_load_pd(srcAccelerationX);
            _mm256_store_pd(srcPosX, _mm256_fmadd_pd(accX, vddt, _mm256_fmsub_pd(posX, two, prevPosX)));
            _mm256_store_pd(srcPrevPosX, posX);
            _mm256_store_pd(srcAccelerationX, zero);

            double* srcPosY = objects.column<1>() + updateI;
            double* srcPrevPosY = objects.column<3>() + updateI;
            double* srcAccelerationY = objects.column<5>() + updateI;
            __m256d posY = _mm256_load_pd(srcPosY);
            __m256d prevPosY = _mm256_load_pd(srcPrevPosY);
            __m256d accY = _mm256_load_pd(srcAccelerationY);
            _mm256_store_pd(srcPosY, _mm256_fmadd_pd(accY, vddt, _mm256_fmsub_pd(posY, two, prevPosY)));
            _mm256_store_pd(srcPrevPosY, posY);
            _mm256_store_pd(srcAccelerationY, zero);
        }
        for (; updateI < objects.size(); updateI++) {
            double tx = objects.column<0>()[updateI];
            objects.column<0>()[updateI] += objects.column<0>()[updateI] - objects.column<2>()[updateI] + objects.column<4>()[updateI] * ddt;
            objects.column<2>()[updateI] = tx;
            objects.column<4>()[updateI] = 0.0;

            double ty = objects.column<1>()[updateI];
            objects.column<1>()[updateI] += objects.column<1>()[updateI] - objects.column<3>()[updateI] + objects.column<5>()[updateI] * ddt;
            objects.column<3>()[updateI] = ty;
            objects.column<5>()[updateI] = 0.0;
        }
        // time[1] = rdtsc();
        if (dynamicColliders.size() == 0) {
            return;
        }
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider2D& col = dynamicColliders[i];
            const uint32_t loc = storage.reg[col.object.ID];
            col.impl.posX = objects.column<0>()[loc] + col.offsetX;
            col.impl.posY = objects.column<1>()[loc] + col.offsetY;
        }
        // time[2] = rdtsc();
        if (dirtyStatic) {
            dirtyStatic = true;
            refreshStaticColliders();
        }
        uint32_t idx = 0u;
        dynamicBounds.ensureCapacity(dynamicColliders.size() * 4u);
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            const DynamicCollider2D& col = dynamicColliders[i];
            dynamicBounds[idx++] = static_cast<float>(col.impl.posX);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posY);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posX + col.impl.sizeX);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posY + col.impl.sizeY);
        }
        // time[3] = rdtsc();
        dynamicBVH.build(dynamicBounds.data(), dynamicColliders.size(),
            {
                .primitiveStride = sizeof(DynamicCollider2D),
                .typeOffset = offsetof(DynamicCollider2D, impl.typeId),
                .dataOffset = offsetof(DynamicCollider2D, impl.userData),
            });
        // time[4] = rdtsc();

        const uint32_t N = dynamicColliders.size() < 8u ? 1u : 8u;
        std::vector<std::thread> threads;
        threads.reserve(N);
        std::vector<std::vector<Collision2D>> result(N);

        // time[5] = rdtsc();

        if (N == 1u) {
            std::vector<Collision2D>& res = result[0u];
            alignas(16u) float query[4u];
            alignas(32u) uint32_t hits[32u];
            CollisionInfo2D info;
            for (uint32_t aIdx = 0u; aIdx < dynamicColliders.size(); aIdx++) {
                DynamicCollider2D& a = dynamicColliders[aIdx];
                _mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                //__m256d v = _mm256_loadu_pd(&a.impl.posX);
                //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(v, _mm256_permute2f128_pd(v, v, 0x00))));
                // const __m256d mask = _mm256_castsi256_pd(_mm256_set_epi64x(-1, -1, 0, 0));
                // alignas(32) double tmp[4] = { a.impl.posX, a.impl.posY, a.impl.sizeX, a.impl.sizeY };
                //__m256d v = _mm256_load_pd(tmp);
                //__m256d A = _mm256_permute_pd(v, 0b0011);
                //__m256d B = _mm256_and_pd(v, mask);
                //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(A, B)));
                uint32_t count = dynamicBVH.query(query, hits, 32u);
                if (count != 0u) {
                    for (uint32_t j = 0u; j < count; j++) {
                        uint32_t bIdx = hits[j];
                        if (aIdx >= bIdx) {
                            continue;
                        }
                        DynamicCollider2D& b = dynamicColliders[bIdx];
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
                            Collider2D& b = staticColliders[bIdx];
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
                    std::vector<Collision2D>& res = result[i];
                    uint32_t* const stack = alloc<uint32_t>(dynamicColliders.size() * 2u);
                    alignas(16u) float query[4u];
                    alignas(32u) uint32_t hits[32u];
                    CollisionInfo2D info;
                    for (uint32_t aIdx = start; aIdx < end; aIdx++) {
                        DynamicCollider2D& a = dynamicColliders[aIdx];
                        _mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                        //__m256d v = _mm256_loadu_pd(&a.impl.posX);
                        //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(v, _mm256_permute2f128_pd(v, v, 0x00))));
                        // const __m256d mask = _mm256_castsi256_pd(_mm256_set_epi64x(-1, -1, 0, 0));
                        // alignas(32) double tmp[4] = { a.impl.posX, a.impl.posY, a.impl.sizeX, a.impl.sizeY };
                        //__m256d v = _mm256_load_pd(tmp);
                        //__m256d A = _mm256_permute_pd(v, 0b0011);
                        //__m256d B = _mm256_and_pd(v, mask);
                        //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(A, B)));
                        uint32_t count = dynamicBVH.query(query, hits, 32u, stack);
                        if (count != 0u) {
                            for (uint32_t j = 0u; j < count; j++) {
                                uint32_t bIdx = hits[j];
                                if (aIdx >= bIdx) {
                                    continue;
                                }
                                DynamicCollider2D& b = dynamicColliders[bIdx];
                                if (!collide(a.impl, b.impl, info)) {
                                    continue;
                                }
                                res.emplace_back(&a, &b, info);
                            }
                        }
                        if (staticColliders.size() > 0u) {
                            count = staticBVH.query(query, hits, 32u, stack);
                            if (count != 0) {
                                for (uint32_t j = 0; j < count; j++) {
                                    uint32_t bIdx = hits[j];
                                    Collider2D& b = staticColliders[bIdx];
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
        for (std::vector<Collision2D>& res : result) {
            for (Collision2D& col : res) {
                DynamicCollider2D* a = col.a;
                uint32_t objA = storage.reg[a->object.ID];
                double& posAX = objects.column<0>()[objA];
                double& posAY = objects.column<1>()[objA];
                double& prevPosAX = objects.column<2>()[objA];
                double& prevPosAY = objects.column<3>()[objA];

                DynamicCollider2D* b = col.b;

                const double normalX = col.normalX;
                const double normalY = col.normalY;
                double penetrationDepth = col.penetrationDepth * resolvingStrength;
                double dx = normalX * penetrationDepth;
                double dy = normalY * penetrationDepth;
                if (b) {
                    double mul = b->mass / (a->mass + b->mass);
                    posAX += dx * mul;
                    posAY += dy * mul;
                    uint32_t objB = storage.reg[b->object.ID];
                    mul = 1.0 - mul;
                    objects.column<0>()[objB] -= dx * mul;
                    objects.column<1>()[objB] -= dy * mul;
                } else {
                    double vx = posAX - prevPosAX;
                    double vy = posAY - prevPosAY;
                    posAX += dx;
                    posAY += dy;
                    double height = -(vx * normalX + vy * normalY);
                    if (height < 0.0) {
                        continue;
                    }
                    double nvx = normalX * height;
                    double nvy = normalY * height;
                    Collider2D& ac = a->impl;
                    double friction = 1.0 - ac.friction;
                    double restitution = ac.restitution;
                    double rx = (vx + nvx) * friction + nvx * restitution;
                    double ry = (vy + nvy) * friction + nvy * restitution;
                    prevPosAX = posAX - rx;
                    prevPosAY = posAY - ry;
                }
            }
        }
        // time[7] = rdtsc();
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider2D& col = dynamicColliders[i];
            uint32_t loc = storage.reg[col.object.ID];
            double posX = objects.column<0>()[loc];
            double posY = objects.column<1>()[loc];
            col.impl.posX = posX + col.offsetX;
            col.impl.posY = posY + col.offsetY;
            if (Transform2D* tx = ecs->getPtr<Transform2D>(objects.column<6>()[loc])) {
                tx->position = prvl::vec2(posX, posY);
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
        Storage<PhysicsObject2D>& storage = ecs->view<PhysicsObject2D>();
        MultiDynamicArray<double, double, double, double, double, double, uint32_t>& objects = storage.objects;

        VirtualArray<DynamicCollider2D>& dynamicColliders = ecs->view<DynamicCollider2D>().data;

        const __m256d zero = _mm256_set1_pd(0.0);
        uint32_t updateI = 0u;
        for (; updateI + 3u < objects.size(); updateI += 4u) {
            double* srcPosX = objects.column<0>() + updateI;
            double* srcPrevPosX = objects.column<2>() + updateI;
            double* srcAccelerationX = objects.column<4>() + updateI;
            _mm256_store_pd(srcPrevPosX, _mm256_load_pd(srcPosX));
            _mm256_store_pd(srcAccelerationX, zero);

            double* srcPosY = objects.column<1>() + updateI;
            double* srcPrevPosY = objects.column<3>() + updateI;
            double* srcAccelerationY = objects.column<5>() + updateI;
            _mm256_store_pd(srcPrevPosY, _mm256_load_pd(srcPosY));
            _mm256_store_pd(srcAccelerationY, zero);
        }
        for (; updateI < objects.size(); updateI++) {
            objects.column<2>()[updateI] = objects.column<0>()[updateI];
            objects.column<4>()[updateI] = 0.0;

            objects.column<3>()[updateI] = objects.column<1>()[updateI];
            objects.column<5>()[updateI] = 0.0;
        }
        for (uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider2D& col = dynamicColliders[i];
            uint32_t loc = storage.reg[col.object.ID];
            double posX = objects.column<0>()[loc];
            double posY = objects.column<1>()[loc];
            col.impl.posX = posX + col.offsetX;
            col.impl.posY = posY + col.offsetY;
            if (Transform2D* tx = ecs->getPtr<Transform2D>(objects.column<6>()[loc])) {
                tx->position = prvl::vec2(posX, posY);
                tx->dirtyTRS = true;
            }
        }
    }
};

template <>
struct Serial<Collider2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        Collider2D* c = world->ecs.getPtr<Collider2D>(componentID);
        output.write(c->typeId);
        if (c->userData) {
            output.write<uint8_t>(1u);
            const ColliderShape2DMetadata& metadata = ColliderShape2DRegistry::colliderMetadata[c->typeId];
            output.write(c->userData, metadata.size);
        } else {
            output.write<uint8_t>(0u);
        }
        output.write(&c->posX, 6u * sizeof(double));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        Collider2D c;
        input.read(c.typeId);
        if (input.read<uint8_t>()) {
            const ColliderShape2DMetadata& metadata = ColliderShape2DRegistry::colliderMetadata[c.typeId];
            c.userData = _mm_malloc(metadata.size, metadata.alignment);
            input.read(c.userData, metadata.size);
        }
        input.read(&c.posX, 6u * sizeof(double));
        e.addComponent(c);
    }
};

template <>
struct Serial<DynamicCollider2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        DynamicCollider2D* c = world->ecs.getPtr<DynamicCollider2D>(componentID);
        output.write(c->impl.typeId);
        if (c->impl.userData) {
            output.write<uint8_t>(1u);
            const ColliderShape2DMetadata& metadata = ColliderShape2DRegistry::colliderMetadata[c->impl.typeId];
            output.write(c->impl.userData, metadata.size);
        } else {
            output.write<uint8_t>(0u);
        }
        output.write(&c->impl.sizeX, 7u * sizeof(double));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        DynamicCollider2D c;
        input.read(c.impl.typeId);
        if (input.read<uint8_t>()) {
            const ColliderShape2DMetadata& metadata = ColliderShape2DRegistry::colliderMetadata[c.impl.typeId];
            c.impl.userData = _mm_malloc(metadata.size, metadata.alignment);
            input.read(c.impl.userData, metadata.size);
        }
        input.read(&c.impl.sizeX, 7u * sizeof(double));
        c.object.ID = UINT32_MAX;
        e.addComponent(c);
    }
};

template <>
struct Serial<PhysicsObject2D> {
    static void serialize(World* world, uint32_t componentID, ByteWriter& output) {
        output.write(world->ecs.read<PhysicsObject2D>(componentID));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(input.read<PhysicsObject2D>());
    }
};

#endif
