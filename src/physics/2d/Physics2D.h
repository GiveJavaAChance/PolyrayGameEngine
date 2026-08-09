#ifndef PHYSICS2D_H_INCLUDED
#define PHYSICS2D_H_INCLUDED

#pragma once

#include <cstdint>
#include <immintrin.h>

#include <iostream>

#include <thread>

#include <structure/BVH.h>
#include <structure/DynamicBVH.h>

#include <structure/DynamicArray.h>

#include <utils/perf.h>

#include <physics/2d/Collider2D.h>
#include <physics/2d/ColliderShape2DMetadata.h>
#include <physics/2d/CollisionInfo2D.h>
#include <physics/2d/DynamicCollider2D.h>
#include <physics/2d/PhysicsObject2D.h>

#include <World.h>

#include <Profiler.h>

using CollisionFunc2D = bool (*)(Collider2D&, void*, Collider2D&, void*, CollisionInfo2D&);

struct Physics2D {
private:
    DynamicArray<ColliderShape2DMetadata> colliderMetadata;

    DynamicArray<CollisionFunc2D> collisionRegistry;

    DynamicArray<float> dynamicBounds;
    DynamicArray<float> staticBounds;

    BVH<2>* staticBVH = nullptr;
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

    template <typename A, typename B, bool (*func)(Collider2D&, A*, Collider2D&, B*, CollisionInfo2D&)>
    inline static bool inverseCollision(Collider2D& a, A* aData, Collider2D& b, B* bData, CollisionInfo2D& infoOut) {
        if (!func(b, reinterpret_cast<A*>(bData), a, reinterpret_cast<B*>(aData), infoOut)) {
            return false;
        }
        infoOut.collisionNormalX = -infoOut.collisionNormalX;
        infoOut.collisionNormalY = -infoOut.collisionNormalY;
        return true;
    }

    bool collide(Collider2D& a, Collider2D& b, CollisionInfo2D& out) {
        uint32_t idA = a.typeId;
        uint32_t idB = b.typeId;
        uint32_t key = makeKey(idA, idB);
        if (key >= collisionRegistry.size()) {
            return false;
        }
        CollisionFunc2D func = collisionRegistry[key];
        if (!func) {
            return false;
        }
        return func(a, a.userData, b, b.userData, out);
    }

    void onStaticColliderAdded(Entity e, uint32_t id) {
        dirtyStatic = true;
    }

    void onStaticColliderRemoved(Entity e, uint32_t id) {
        dirtyStatic = true;
    }

    void onDynamicColliderAdded(Entity e, uint32_t id) {
        uint32_t objID;
        if (ecs->getComponentID<PhysicsObject2D>(e.entityID, objID)) {
            ecs->getPtr<DynamicCollider2D>(id)->object.ID = objID;
        }
    }

    void onDynamicColliderRemoved(Entity e, uint32_t id) {
        dirtyStatic = true;
    }

    void refreshStaticColliders() {
        dirtyStatic = false;
        DynamicArray<Collider2D>& staticColliders = ecs->view<Collider2D>().data;
        if (staticColliders.size() == 0) {
            if (staticBVH) {
                delete staticBVH;
            }
            return;
        }
        int idx = 0;
        staticBounds.ensureCapacity(staticColliders.size() * 4);
        for (uint32_t i = 0u; i < staticColliders.size(); i++) {
            const Collider2D& col = staticColliders[i];
            staticBounds[idx++] = static_cast<float>(col.posX);
            staticBounds[idx++] = static_cast<float>(col.posY);
            staticBounds[idx++] = static_cast<float>(col.posX + col.sizeX);
            staticBounds[idx++] = static_cast<float>(col.posY + col.sizeY);
        }
        if (staticBVH) {
            delete staticBVH;
        }
        staticBVH = new BVH<2>(staticBounds.data(), staticColliders.size());
    }

    template <typename T>
    void registerColliderShape() {
        ColliderShape2DMetadata metadata = ColliderShape2DMetadata::get<T>();
        while (metadata.typeId >= colliderMetadata.size()) {
            colliderMetadata.add(ColliderShape2DMetadata::invalid());
        }
        colliderMetadata[metadata.typeId] = metadata;
    }

public:
    Physics2D(ECS* ecs, bool disabled) : ecs(ecs) {
        ecs->registerComponentListener<Collider2D, Physics2D, onStaticColliderAdded, onStaticColliderRemoved>(this);
        ecs->registerComponentListener<DynamicCollider2D, Physics2D, onDynamicColliderAdded, onDynamicColliderRemoved>(this);
        if (disabled) {
            ecs->registerUpdateCallback<Physics2D, disabledPhysicsUpdate, UpdateOrder::PHYSICS>(this);
        } else {
            ecs->registerUpdateCallback<Physics2D, physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
    }

    const ColliderShape2DMetadata& getColliderShapeMetadata(uint32_t type) {
        return colliderMetadata[type];
    }

    template <typename A, typename B, bool (*func)(Collider2D&, A*, Collider2D&, B*, CollisionInfo2D&)>
    void registerCollision() {
        registerColliderShape<A>();
        registerColliderShape<B>();
        uint32_t idA = ColliderShape2DMetadata::typeOf<A>();
        uint32_t idB = ColliderShape2DMetadata::typeOf<B>();
        uint32_t keyAB = makeKey(idA, idB);
        uint32_t keyBA = makeKey(idB, idA);
        uint32_t maxKey = std::max(keyAB, keyBA);
        if (maxKey >= collisionRegistry.size()) {
            collisionRegistry.reserve(maxKey + 1u - collisionRegistry.size());
        }
        collisionRegistry[keyAB] = reinterpret_cast<CollisionFunc2D>(func);
        if (idA != idB) {
            collisionRegistry[keyBA] = reinterpret_cast<CollisionFunc2D>(&inverseCollision<A, B, func>);
        }
    }

    inline void markDirtyStatic() {
        dirtyStatic = true;
    }

    template <typename T>
    inline Collider2D createCollider(T* userData, double posX, double posY, double sizeX, double sizeY, double friction, double restitution) {
        return Collider2D{ColliderShape2DMetadata::typeOf<T>(), userData, posX, posY, sizeX, sizeY, friction, restitution};
    }

    void physicsUpdate(double dt) {
        PROFILE_SCOPE(Physics2DUpdate)
        Storage<PhysicsObject2D>& storage = ecs->view<PhysicsObject2D>();
        MultiDynamicArray<double, double, double, double, double, double>& objects = storage.objects;

        DynamicArray<Collider2D>& staticColliders = ecs->view<Collider2D>().data;
        DynamicArray<DynamicCollider2D>& dynamicColliders = ecs->view<DynamicCollider2D>().data;

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
        dynamicBVH.build(dynamicBounds.data(), dynamicColliders.size());
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
                if (staticBVH) {
                    count = staticBVH->query(query, hits, 32u);
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
                        if (staticBVH) {
                            count = staticBVH->query(query, hits, 32u, stack);
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
            const uint32_t loc = storage.reg[col.object.ID];
            col.impl.posX = objects.column<0>()[loc] + col.offsetX;
            col.impl.posY = objects.column<1>()[loc] + col.offsetY;
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
        MultiDynamicArray<double, double, double, double, double, double>& objects = storage.objects;

        DynamicArray<DynamicCollider2D>& dynamicColliders = ecs->view<DynamicCollider2D>().data;

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
            const uint32_t loc = storage.reg[col.object.ID];
            col.impl.posX = objects.column<0>()[loc] + col.offsetX;
            col.impl.posY = objects.column<1>()[loc] + col.offsetY;
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
            Physics2D* physicsSystem = world->getSystem<Physics2D>();
            const ColliderShape2DMetadata& metadata = physicsSystem->getColliderShapeMetadata(c->typeId);
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
            Physics2D* physicsSystem = world->getSystem<Physics2D>();
            const ColliderShape2DMetadata& metadata = physicsSystem->getColliderShapeMetadata(c.typeId);
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
            Physics2D* physicsSystem = world->getSystem<Physics2D>();
            const ColliderShape2DMetadata& metadata = physicsSystem->getColliderShapeMetadata(c->impl.typeId);
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
            Physics2D* physicsSystem = world->getSystem<Physics2D>();
            const ColliderShape2DMetadata& metadata = physicsSystem->getColliderShapeMetadata(c.impl.typeId);
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
        output.write(world->ecs.read<PhysicsObject2D>({componentID}));
    }

    static void deserialize(World* world, Entity& e, ByteReader& input) {
        e.addComponent(input.read<PhysicsObject2D>());
    }
};

#endif
