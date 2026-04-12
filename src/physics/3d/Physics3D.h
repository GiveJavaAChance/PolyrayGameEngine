#ifndef PHYSICS3D_H_INCLUDED
#define PHYSICS3D_H_INCLUDED

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <immintrin.h>

#include <iostream>

#include <thread>
#include <vector>

#include <structure/BVH.h>
#include <structure/StaticRegistry.h>
#include <structure/DynamicArray.h>
#include <structure/MultiDynamicArray.h>
#include <typereg.h>

#include <utils/perf.h>

#include <physics/3d/CollisionInfo3D.h>
#include <physics/3d/Collider3D.h>
#include <physics/3d/DynamicCollider3D.h>
#include <physics/3d/PhysicsObject3D.h>

#include <ecs/ECS.h>

using CollisionFunc3D = bool(*)(Collider3D&, void*, Collider3D&, void*, CollisionInfo3D&);

template<>
struct Storage<PhysicsObject3D, StorageDataLayout::CUSTOM> {
    MultiDynamicArray<double, double, double, double, double, double, double, double, double> objects;
    Registry reg;

    inline uint32_t add(const PhysicsObject3D& component) noexcept {
        objects.add(
            component.posX, component.posY, component.posZ,
            component.prevPosX, component.prevPosY, component.prevPosZ,
            component.accX, component.accY, component.accZ
        );
        return reg.create();
    }

    inline void set(uint32_t componentID, const PhysicsObject3D& component) noexcept {
        objects.setIndex(reg[componentID],
            component.posX, component.posY, component.posZ,
            component.prevPosX, component.prevPosY, component.prevPosZ,
            component.accX, component.accY, component.accZ
        );
    }

    inline void remove(uint32_t componentID) noexcept {
        uint32_t loc;
        if(reg.remove(componentID, loc)) {
            objects.set(loc, objects, objects.size() - 1u);
        }
        objects.removeEnd();
    }

    inline PhysicsObject3D get(uint32_t id) const noexcept {
        uint32_t loc = reg[id];
        return PhysicsObject3D{
            objects.column<0>()[loc],
            objects.column<1>()[loc],
            objects.column<2>()[loc],
            objects.column<3>()[loc],
            objects.column<4>()[loc],
            objects.column<5>()[loc],
            objects.column<6>()[loc],
            objects.column<7>()[loc],
            objects.column<8>()[loc]
        };
    }
};

struct Physics3D {
private:
    DynamicArray<CollisionFunc3D> collisionRegistry;
    DynamicArray<void(*)(BVH<3>&)> queryCallbacks;

    DynamicArray<float> dynamicBounds;
    DynamicArray<float> staticBounds;

    BVH<3>* staticBVH = nullptr;
    bool dirtyStatic = true;

    double resolvingStrength = 0.5;

    ECS* ecs;

    struct Collision3D {
        DynamicCollider3D* a;
        DynamicCollider3D* b;
        Collider3D* bc;
        const double normalX, normalY, normalZ;
        const double penetrationDepth;

        Collision3D(DynamicCollider3D* a, DynamicCollider3D* b, CollisionInfo3D& info) : a(a), b(b), normalX(info.collisionNormalX), normalY(info.collisionNormalY), normalZ(info.collisionNormalZ), penetrationDepth(info.penetrationDepth) {
            this->bc = nullptr;
        }

        Collision3D(DynamicCollider3D* a, Collider3D* b, CollisionInfo3D& info) : a(a), bc(b), normalX(info.collisionNormalX), normalY(info.collisionNormalY), normalZ(info.collisionNormalZ), penetrationDepth(info.penetrationDepth) {
            this->b = nullptr;
        }
    };

    TYPE_REGISTRY(ColliderTypes)

    inline static uint32_t makeKey(uint32_t a, uint32_t b) {
        return (((a + b) * (a + b + 1)) >> 1u) + b;
    }

    template <typename A, typename B, bool(*func)(Collider3D&, A*, Collider3D&, B*, CollisionInfo3D&)>
    inline static bool inverseCollision(Collider3D& a, A* aData, Collider3D& b, B* bData, CollisionInfo3D& infoOut) {
        if (!func(b, reinterpret_cast<A*>(bData), a, reinterpret_cast<B*>(aData), infoOut)) {
            return false;
        }
        infoOut.collisionNormalX = -infoOut.collisionNormalX;
        infoOut.collisionNormalY = -infoOut.collisionNormalY;
        infoOut.collisionNormalZ = -infoOut.collisionNormalZ;
        return true;
    }

    bool collide(Collider3D& a, Collider3D& b, CollisionInfo3D& out) {
        uint32_t idA = a.typeId;
        uint32_t idB = b.typeId;
        uint32_t key = makeKey(idA, idB);
        if(key >= collisionRegistry.size()) {
            return false;
        }
        CollisionFunc3D func = collisionRegistry[key];
        if(!func) {
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

public:
    Physics3D(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentType<PhysicsObject3D>();
        ecs->registerComponentType<Collider3D>();
        ecs->registerComponentType<DynamicCollider3D>();
        ecs->registerUpdateCallback<Physics3D, physicsUpdate, UpdateOrder::PHYSICS>(this);
        ecs->registerComponentListener<Collider3D, Physics3D, onStaticColliderAdded, onStaticColliderRemoved>(this);
    }

    template<typename A, typename B, bool(*func)(Collider3D&, A*, Collider3D&, B*, CollisionInfo3D&)>
    void registerCollision() {
        uint32_t idA = ColliderTypes::getTypeId<A>();
        uint32_t idB = ColliderTypes::getTypeId<B>();
        uint32_t keyAB = makeKey(idA, idB);
        uint32_t keyBA = makeKey(idB, idA);
        uint32_t maxKey = std::max(keyAB, keyBA);
        if (maxKey >= collisionRegistry.size()) {
            collisionRegistry.reserve(maxKey + 1u - collisionRegistry.size());
        }
        collisionRegistry[keyAB] = reinterpret_cast<CollisionFunc3D>(func);
        if(idA != idB) {
            collisionRegistry[keyBA] = reinterpret_cast<CollisionFunc3D>(&inverseCollision<A, B, func>);
        }
    }

    inline void addDynamicQueryCallback(void(*func)(BVH<3>&)) {
        queryCallbacks.add(static_cast<void(*)(BVH<3>&)>(func));
    }

    void refreshStaticColliders() {
        dirtyStatic = false;
        DynamicArray<Collider3D>& staticColliders = ecs->view<Collider3D>().data;
        if(staticColliders.size() == 0) {
            if(staticBVH) {
                delete staticBVH;
            }
            return;
        }
        int idx = 0;
        staticBounds.ensureCapacity(staticColliders.size() * 6);
        for (uint32_t i = 0u; i < staticColliders.size(); i++) {
            const Collider3D& col = staticColliders[i];
            staticBounds[idx++] = static_cast<float>(col.posX);
            staticBounds[idx++] = static_cast<float>(col.posY);
            staticBounds[idx++] = static_cast<float>(col.posZ);
            staticBounds[idx++] = static_cast<float>(col.posX + col.sizeX);
            staticBounds[idx++] = static_cast<float>(col.posY + col.sizeY);
            staticBounds[idx++] = static_cast<float>(col.posZ + col.sizeZ);
        }
        if(staticBVH) {
            delete staticBVH;
        }
        staticBVH = new BVH<3>(staticBounds.data(), staticColliders.size());
    }

    template<typename T>
    inline Collider3D createCollider(void* userData, double posX, double posY, double posZ, double sizeX, double sizeY, double sizeZ, double friction, double restitution) {
        return Collider3D{ColliderTypes::getTypeId<T>(), userData, posX, posY, posZ, sizeX, sizeY, sizeZ, 1.0 - friction, restitution};
    }

    void physicsUpdate(const double dt) {
        Storage<PhysicsObject3D, StorageDataLayout::CUSTOM>& storage = ecs->view<PhysicsObject3D>();
        MultiDynamicArray<double, double, double, double, double, double, double, double, double>& objects = storage.objects;

        DynamicArray<Collider3D>& staticColliders = ecs->view<Collider3D>().data;
        DynamicArray<DynamicCollider3D>& dynamicColliders = ecs->view<DynamicCollider3D>().data;

        //uint64_t time[9];
        //time[0] = rdtsc();
        const double ddt = dt * dt;
        const __m256d vddt = _mm256_broadcast_sd(&ddt); // _mm256_set1_pd(ddt);
        const __m256d zero = _mm256_set1_pd(0.0);
        const __m256d two = _mm256_set1_pd(2.0);
        uint32_t updateI = 0u;
        for (; updateI + 5u < objects.size(); updateI += 6u) {
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
        for(; updateI < objects.size(); updateI++) {
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
        //time[1] = rdtsc();
        if (dynamicColliders.size() == 0) {
            return;
        }
        for(uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider3D& col = dynamicColliders[i];
            const uint32_t loc = storage.reg[col.object.ID];
            col.impl.posX = objects.column<0>()[loc] + col.offsetX;
            col.impl.posY = objects.column<1>()[loc] + col.offsetY;
            col.impl.posZ = objects.column<2>()[loc] + col.offsetZ;
        }
        //time[2] = rdtsc();
        if (dirtyStatic) {
            refreshStaticColliders();
        }
        uint32_t idx = 0u;
        dynamicBounds.ensureCapacity(dynamicColliders.size() * 6u);
        for(uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            const DynamicCollider3D& col = dynamicColliders[i];
            dynamicBounds[idx++] = static_cast<float>(col.impl.posX);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posY);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posZ);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posX + col.impl.sizeX);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posY + col.impl.sizeY);
            dynamicBounds[idx++] = static_cast<float>(col.impl.posZ + col.impl.sizeZ);
        }
        //time[3] = rdtsc();
        BVH<3> dynamicBVH(dynamicBounds.data(), dynamicColliders.size());
        //time[4] = rdtsc();
        for(uint32_t i = 0; i < queryCallbacks.size(); i++) {
            queryCallbacks[i](dynamicBVH);
        }

        const uint32_t N = dynamicColliders.size() < 8u ? 1u : 8u;
        std::vector<std::thread> threads;
        threads.reserve(N);
        std::vector<std::vector<Collision3D>> result(N);

        //time[5] = rdtsc();

        if (N == 1u) {
            std::vector<Collision3D>& res = result[0u];
            alignas(16) float query[6];
            alignas(32) int hits[32];
            CollisionInfo3D info;
            for (uint32_t aIdx = 0u; aIdx < dynamicColliders.size(); aIdx++) {
                DynamicCollider3D& a = dynamicColliders[aIdx];
                //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                query[0] = static_cast<float>(a.impl.posX);
                query[1] = static_cast<float>(a.impl.posY);
                query[2] = static_cast<float>(a.impl.posZ);
                query[3] = static_cast<float>(a.impl.posX + a.impl.sizeX);
                query[4] = static_cast<float>(a.impl.posY + a.impl.sizeY);
                query[5] = static_cast<float>(a.impl.posZ + a.impl.sizeZ);
                uint32_t count = dynamicBVH.query(query, hits, 32);
                if (count != 0) {
                    for (uint32_t j = 0; j < count; j++) {
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
                if(staticBVH) {
                    count = staticBVH->query(query, hits, 32);
                    if (count != 0) {
                        for (uint32_t j = 0; j < count; j++) {
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
            for (uint32_t i = 0; i < N; i++) {
                uint32_t start = i * dynamicColliders.size() / N;
                uint32_t end = (i + 1) * dynamicColliders.size() / N;
                threads.emplace_back([&, i, start, end]() {
                    std::vector<Collision3D>& res = result[i];
                    int* const stack = alloc<int>(dynamicColliders.size() * 2);
                    alignas(16) float query[6];
                    alignas(32) int hits[32];
                    CollisionInfo3D info;
                    for (uint32_t aIdx = start; aIdx < end; aIdx++) {
                        DynamicCollider3D& a = dynamicColliders[aIdx];
                        //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                        query[0] = static_cast<float>(a.impl.posX);
                        query[1] = static_cast<float>(a.impl.posY);
                        query[2] = static_cast<float>(a.impl.posZ);
                        query[3] = static_cast<float>(a.impl.posX + a.impl.sizeX);
                        query[4] = static_cast<float>(a.impl.posY + a.impl.sizeY);
                        query[5] = static_cast<float>(a.impl.posZ + a.impl.sizeZ);
                        uint32_t count = dynamicBVH.query(query, hits, 32, stack);
                        if (count != 0) {
                            for (uint32_t j = 0; j < count; j++) {
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
                        if(staticBVH) {
                            count = staticBVH->query(query, hits, 32, stack);
                            if (count != 0) {
                                for (uint32_t j = 0; j < count; j++) {
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
        //time[6] = rdtsc();
        for(std::vector<Collision3D>& res : result) {
            for(Collision3D& col : res) {
                DynamicCollider3D* a = col.a;
                uint32_t objA = storage.reg[a->object.ID];
                double& posAX = objects.column<0>()[objA];
                double& posAY = objects.column<1>()[objA];
                double& posAZ = objects.column<2>()[objA];
                double& prevPosAX = objects.column<3>()[objA];
                double& prevPosAY = objects.column<4>()[objA];
                double& prevPosAZ = objects.column<5>()[objA];

                DynamicCollider3D* b = col.b;

                const double normalX = col.normalX;
                const double normalY = col.normalY;
                const double normalZ = col.normalZ;
                double penetrationDepth = col.penetrationDepth * resolvingStrength;
                if (b) {
                    penetrationDepth *= 0.5;
                }
                double dx = normalX * penetrationDepth;
                double dy = normalY * penetrationDepth;
                double dz = normalZ * penetrationDepth;
                if(b) {
                    posAX += dx;
                    posAY += dy;
                    posAZ += dz;
                    uint32_t objB = storage.reg[b->object.ID];
                    objects.column<0>()[objB] -= dx;
                    objects.column<1>()[objB] -= dy;
                    objects.column<2>()[objB] -= dz;
                } else {
                    double vx = posAX - prevPosAX;
                    double vy = posAY - prevPosAY;
                    double vz = posAZ - prevPosAZ;
                    posAX += dx;
                    posAY += dy;
                    posAZ += dz;
                    double height = -(vx * normalX + vy * normalY + vz * normalZ);
                    if (height < 0.0) {
                        continue;
                    }
                    double nvx = normalX * height;
                    double nvy = normalY * height;
                    double nvz = normalZ * height;
                    Collider3D& ac = a->impl;
                    double friction = ac.friction;
                    double restitution = ac.restitution;
                    double rx = (vx + nvx) * friction + nvx * restitution;
                    double ry = (vy + nvy) * friction + nvy * restitution;
                    double rz = (vz + nvz) * friction + nvz * restitution;
                    prevPosAX = posAX - rx;
                    prevPosAY = posAY - ry;
                    prevPosAZ = posAZ - rz;
                }
            }
        }
        //time[7] = rdtsc();
        for(uint32_t i = 0u; i < dynamicColliders.size(); i++) {
            DynamicCollider3D& col = dynamicColliders[i];
            const uint32_t loc = storage.reg[col.object.ID];
            col.impl.posX = objects.column<0>()[loc] + col.offsetX;
            col.impl.posY = objects.column<1>()[loc] + col.offsetY;
            col.impl.posZ = objects.column<2>()[loc] + col.offsetZ;
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
};

#endif
