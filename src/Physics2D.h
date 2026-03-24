#ifndef PHYSICS2D_H_INCLUDED
#define PHYSICS2D_H_INCLUDED

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <immintrin.h>

#include <iostream>

#include <thread>
#include <vector>

#include "BVH.h"
#include "StaticRegistry.h"
#include "DynamicArray.h"
#include "MultiDynamicArray.h"
#include "typereg.h"

#include "perf.h"

struct CollisionInfo2D {
    double collisionNormalX;
    double collisionNormalY;
    double penetrationDepth;
};

struct Collider2D {
    uint32_t typeId;
    void* userData;
    double posX, posY;
    double sizeX, sizeY;
    double friction, restitution;
};

struct DynamicCollider2D {
    uint32_t objectID;
    Collider2D impl;
    double offsetX, offsetY;
};

using CollisionFunc2D = bool(*)(Collider2D&, void*, Collider2D&, void*, CollisionInfo2D&);

struct PhysicsObject2D;

namespace Physics2D {
    namespace Internal {
        /*inline DynamicArray<double> objPosX;
        inline DynamicArray<double> objPosY;
        inline DynamicArray<double> objPrevPosX;
        inline DynamicArray<double> objPrevPosY;
        inline DynamicArray<double> objAccelerationX;
        inline DynamicArray<double> objAccelerationY;*/

        /*inline Registry<[](const uint32_t dst, const uint32_t src) {
            objPosX[dst] = objPosX[src];
            objPosY[dst] = objPosY[src];
            objPrevPosX[dst] = objPrevPosX[src];
            objPrevPosY[dst] = objPrevPosY[src];
            objAccelerationX[dst] = objAccelerationX[src];
            objAccelerationY[dst] = objAccelerationY[src];
        }> objectReg;*/

        inline MultiDynamicArray<double, double, double, double, double, double> objects;
        inline void moveObject(const uint32_t dst, const uint32_t src) {
            objects.set(dst, objects, src);
        }
        inline StaticRegistry<moveObject> objectReg;

        inline std::vector<DynamicCollider2D> dynamicColliders;
        inline void moveDynamicCollider(const uint32_t dst, const uint32_t src) {
            std::memcpy(dynamicColliders.data() + dst, dynamicColliders.data() + src, sizeof(DynamicCollider2D));
        }
        inline StaticRegistry<moveDynamicCollider> dynamicColliderReg;

        //inline MultiDynamicArray<uint32_t, uint32_t, void*, double, double, double, double, double, double, double, double> dynamicColliders;
        //inline Registry<[](const uint32_t dst, const uint32_t src){dynamicColliders.set(dst, dynamicColliders, src);}> dynamicColliderReg;

        inline std::vector<Collider2D> staticColliders;
        inline void moveStaticCollider(const uint32_t dst, const uint32_t src) {
            std::memcpy(staticColliders.data() + dst, staticColliders.data() + src, sizeof(Collider2D));
        }
        inline StaticRegistry<moveStaticCollider> staticColliderReg;

        inline DynamicArray<CollisionFunc2D> collisionRegistry;

        inline DynamicArray<void(*)(BVH<2>&)> queryCallbacks;

        inline DynamicArray<float> dynamicBounds;
        inline DynamicArray<float> staticBounds;

        inline BVH<2>* staticBVH = nullptr;

        inline bool dirtyStatic = true;

        static constexpr double resolvingStrength = 0.5;

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

        TYPE_REGISTRY(ColliderTypes)

        inline uint32_t makeKey(uint32_t a, uint32_t b) {
            return (((a + b) * (a + b + 1)) >> 1u) + b;
        }

        template <typename A, typename B, bool(*func)(Collider2D&, A*, Collider2D&, B*, CollisionInfo2D&)>
        inline bool inverseCollision(Collider2D& a, A* aData, Collider2D& b, B* bData, CollisionInfo2D& infoOut) {
            if (!func(b, reinterpret_cast<A*>(bData), a, reinterpret_cast<B*>(aData), infoOut)) {
                return false;
            }
            infoOut.collisionNormalX = -infoOut.collisionNormalX;
            infoOut.collisionNormalY = -infoOut.collisionNormalY;
            return true;
        }

        inline bool collide(Collider2D& a, Collider2D& b, CollisionInfo2D& out) {
            uint32_t idA = a.typeId;
            uint32_t idB = b.typeId;
            uint32_t key = makeKey(idA, idB);
            if(key >= collisionRegistry.size()) {
                return false;
            }
            CollisionFunc2D func = collisionRegistry[key];
            if(!func) {
                return false;
            }
            return func(a, a.userData, b, b.userData, out);
        }
    }

    using namespace Internal;

    template<typename A, typename B, bool(*func)(Collider2D&, A*, Collider2D&, B*, CollisionInfo2D&)>
    inline void registerCollision() {
        uint32_t idA = ColliderTypes::getTypeId<A>();
        uint32_t idB = ColliderTypes::getTypeId<B>();
        uint32_t keyAB = makeKey(idA, idB);
        uint32_t keyBA = makeKey(idB, idA);
        uint32_t maxKey = std::max(keyAB, keyBA);
        if (maxKey >= collisionRegistry.size()) {
            collisionRegistry.reserve(maxKey + 1u - collisionRegistry.size());
        }
        collisionRegistry[keyAB] = reinterpret_cast<CollisionFunc2D>(func);
        if(idA != idB) {
            collisionRegistry[keyBA] = reinterpret_cast<CollisionFunc2D>(&inverseCollision<A, B, func>);
        }
    }

    inline void addDynamicQueryCallback(void(*func)(BVH<2>&)) {
        queryCallbacks.add(static_cast<void(*)(BVH<2>&)>(func));
    }

    inline PhysicsObject2D createObject(const double posX, const double posY, const double velX = 0.0, const double velY = 0.0, const double accelerationX = 0.0, const double accelerationY = 0.0);

    inline void deleteObject(const uint32_t objectID);

    template<typename T>
    inline Collider2D createCollider(void* const userData, const double posX, const double posY, const double sizeX, const double sizeY, const double friction = 0.0, const double restitution = 0.0);

    inline uint32_t createDynamicCollider(const uint32_t objectID, const Collider2D& col, const double offsetX, const double offsetY);

    inline void deleteDynamicCollider(const uint32_t colliderID);

    inline DynamicCollider2D& getDynamicCollider(const uint32_t colliderID);

    inline uint32_t createStaticCollider(const Collider2D& col);

    inline void deleteStaticCollider(const uint32_t colliderID);

    inline Collider2D& getStaticCollider(const uint32_t colliderID);

    inline void refreshStaticColliders() {
        dirtyStatic = false;
        if(staticColliders.size() == 0) {
            if(staticBVH) {
                delete staticBVH;
            }
            return;
        }
        int idx = 0;
        staticBounds.ensureCapacity(staticColliders.size() * 4);
        for (Collider2D& col : staticColliders) {
            staticBounds[idx++] = static_cast<float>(col.posX);
            staticBounds[idx++] = static_cast<float>(col.posY);
            staticBounds[idx++] = static_cast<float>(col.posX + col.sizeX);
            staticBounds[idx++] = static_cast<float>(col.posY + col.sizeY);
        }
        if(staticBVH) {
            delete staticBVH;
        }
        staticBVH = new BVH<2>(staticBounds.data(), staticColliders.size());
    }

    inline void physicsUpdate(const double dt);
}

struct PhysicsObject2D {
    uint32_t objectID;

    constexpr PhysicsObject2D(uint32_t ID) : objectID(ID) {
    }

    inline double& posX() const {
        return Physics2D::Internal::objects.column<0>()[Physics2D::Internal::objectReg[objectID]];
    }

    inline double& posY() const {
        return Physics2D::Internal::objects.column<1>()[Physics2D::Internal::objectReg[objectID]];
    }

    inline double& prevPosX() const {
        return Physics2D::Internal::objects.column<2>()[Physics2D::Internal::objectReg[objectID]];
    }

    inline double& prevPosY() const {
        return Physics2D::Internal::objects.column<3>()[Physics2D::Internal::objectReg[objectID]];
    }

    inline double& accX() const {
        return Physics2D::Internal::objects.column<4>()[Physics2D::Internal::objectReg[objectID]];
    }

    inline double& accY() const {
        return Physics2D::Internal::objects.column<5>()[Physics2D::Internal::objectReg[objectID]];
    }
};

inline PhysicsObject2D Physics2D::createObject(const double posX, const double posY, const double velX, const double velY, const double accelerationX, const double accelerationY) {
    const uint32_t ID = objectReg.create();
    objects.add(posX, posY, posX - velX, posY - velY, accelerationX, accelerationY);
    return PhysicsObject2D(ID);
}

inline void Physics2D::deleteObject(const uint32_t objectID) {
    objectReg.remove(objectID);
    objects.removeEnd();
}

template<typename T>
inline Collider2D Physics2D::createCollider(void* const userData, const double posX, const double posY, const double sizeX, const double sizeY, const double friction, const double restitution) {
    return Collider2D{ColliderTypes::getTypeId<T>(), userData, posX, posY, sizeX, sizeY, 1.0 - friction, restitution};
}

inline uint32_t Physics2D::createDynamicCollider(const uint32_t objectID, const Collider2D& col, const double offsetX, const double offsetY) {
    dynamicColliders.emplace_back(objectID, col, offsetX, offsetY);
    return dynamicColliderReg.create();
}

inline void Physics2D::deleteDynamicCollider(const uint32_t colliderID) {
    dynamicColliderReg.remove(colliderID);
    dynamicColliders.pop_back();
}

inline DynamicCollider2D& Physics2D::getDynamicCollider(const uint32_t colliderID) {
    return dynamicColliders[dynamicColliderReg[colliderID]];
}

inline uint32_t Physics2D::createStaticCollider(const Collider2D& col) {
    staticColliders.push_back(col);
    dirtyStatic = true;
    return staticColliderReg.create();
}

inline void Physics2D::deleteStaticCollider(const uint32_t colliderID) {
    staticColliderReg.remove(colliderID);
    staticColliders.pop_back();
    dirtyStatic = true;
}

inline Collider2D& Physics2D::getStaticCollider(const uint32_t colliderID) {
    return staticColliders[staticColliderReg[colliderID]];
}

inline void Physics2D::physicsUpdate(const double dt) {
    //uint64_t time[9];
    //time[0] = rdtsc();
    const double ddt = dt * dt;
    const __m256d vddt = _mm256_broadcast_sd(&ddt); // _mm256_set1_pd(ddt);
    const __m256d zero = _mm256_set1_pd(0.0);
    const __m256d two = _mm256_set1_pd(2.0);
    uint32_t updateI = 0u;
    for (; updateI + 3u < objectReg.size(); updateI += 4) {
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
    for(; updateI < objectReg.size(); updateI++) {
        double tx = objects.column<0>()[updateI];
        objects.column<0>()[updateI] += objects.column<0>()[updateI] - objects.column<2>()[updateI] + objects.column<4>()[updateI] * ddt;
        objects.column<2>()[updateI] = tx;
        objects.column<4>()[updateI] = 0.0;

        double ty = objects.column<1>()[updateI];
        objects.column<1>()[updateI] += objects.column<1>()[updateI] - objects.column<3>()[updateI] + objects.column<5>()[updateI] * ddt;
        objects.column<3>()[updateI] = ty;
        objects.column<5>()[updateI] = 0.0;
    }
    //time[1] = rdtsc();
    if (dynamicColliders.size() == 0) {
        return;
    }
    for(DynamicCollider2D& col : dynamicColliders) {
        const uint32_t loc = objectReg[col.objectID];
        col.impl.posX = objects.column<0>()[loc] + col.offsetX;
        col.impl.posY = objects.column<1>()[loc] + col.offsetY;
    }
    //time[2] = rdtsc();
    if (dirtyStatic) {
        refreshStaticColliders();
    }
    uint32_t idx = 0u;
    dynamicBounds.ensureCapacity(dynamicColliders.size() * 4u);
    for (DynamicCollider2D& col : dynamicColliders) {
        dynamicBounds[idx++] = static_cast<float>(col.impl.posX);
        dynamicBounds[idx++] = static_cast<float>(col.impl.posY);
        dynamicBounds[idx++] = static_cast<float>(col.impl.posX + col.impl.sizeX);
        dynamicBounds[idx++] = static_cast<float>(col.impl.posY + col.impl.sizeY);
    }
    //time[3] = rdtsc();
    BVH<2> dynamicBVH(dynamicBounds.data(), dynamicColliders.size());
    //time[4] = rdtsc();
    for(uint32_t i = 0; i < queryCallbacks.size(); i++) {
        queryCallbacks[i](dynamicBVH);
    }

    const uint32_t N = dynamicColliders.size() < 8u ? 1u : 8u;
    std::vector<std::thread> threads;
    threads.reserve(N);
    std::vector<std::vector<Collision2D>> result(N);

    //time[5] = rdtsc();

    if (N == 1u) {
        std::vector<Collision2D>& res = result[0u];
        alignas(16) float query[4];
        alignas(32) int hits[32];
        CollisionInfo2D info;
        for (uint32_t aIdx = 0u; aIdx < dynamicColliders.size(); aIdx++) {
            DynamicCollider2D& a = dynamicColliders[aIdx];
            _mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
            //__m256d v = _mm256_loadu_pd(&a.impl.posX);
            //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(v, _mm256_permute2f128_pd(v, v, 0x00))));
            //const __m256d mask = _mm256_castsi256_pd(_mm256_set_epi64x(-1, -1, 0, 0));
            //alignas(32) double tmp[4] = { a.impl.posX, a.impl.posY, a.impl.sizeX, a.impl.sizeY };
            //__m256d v = _mm256_load_pd(tmp);
            //__m256d A = _mm256_permute_pd(v, 0b0011);
            //__m256d B = _mm256_and_pd(v, mask);
            //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(A, B)));
            uint32_t count = dynamicBVH.query(query, hits, 32);
            if (count != 0) {
                for (uint32_t j = 0; j < count; j++) {
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
            if(staticBVH) {
                count = staticBVH->query(query, hits, 32);
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
    } else {
        for (uint32_t i = 0; i < N; i++) {
            uint32_t start = i * dynamicColliders.size() / N;
            uint32_t end = (i + 1) * dynamicColliders.size() / N;
            threads.emplace_back([&, i, start, end]() {
                std::vector<Collision2D>& res = result[i];
                int* const stack = alloc<int>(dynamicColliders.size() * 2);
                alignas(16) float query[4];
                alignas(32) int hits[32];
                CollisionInfo2D info;
                for (uint32_t aIdx = start; aIdx < end; aIdx++) {
                    DynamicCollider2D& a = dynamicColliders[aIdx];
                    _mm_store_ps(query, _mm256_cvtpd_ps(_mm256_set_pd(a.impl.posY + a.impl.sizeY, a.impl.posX + a.impl.sizeX, a.impl.posY, a.impl.posX)));
                    //__m256d v = _mm256_loadu_pd(&a.impl.posX);
                    //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(v, _mm256_permute2f128_pd(v, v, 0x00))));
                    //const __m256d mask = _mm256_castsi256_pd(_mm256_set_epi64x(-1, -1, 0, 0));
                    //alignas(32) double tmp[4] = { a.impl.posX, a.impl.posY, a.impl.sizeX, a.impl.sizeY };
                    //__m256d v = _mm256_load_pd(tmp);
                    //__m256d A = _mm256_permute_pd(v, 0b0011);
                    //__m256d B = _mm256_and_pd(v, mask);
                    //_mm_store_ps(query, _mm256_cvtpd_ps(_mm256_add_pd(A, B)));
                    uint32_t count = dynamicBVH.query(query, hits, 32, stack);
                    if (count != 0) {
                        for (uint32_t j = 0; j < count; j++) {
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
                    if(staticBVH) {
                        count = staticBVH->query(query, hits, 32, stack);
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
    //time[6] = rdtsc();
    for(std::vector<Collision2D>& res : result) {
        for(Collision2D& col : res) {
            DynamicCollider2D* a = col.a;
            uint32_t objA = objectReg[a->objectID];
            double& posAX = objects.column<0>()[objA];
            double& posAY = objects.column<1>()[objA];
            double& prevPosAX = objects.column<2>()[objA];
            double& prevPosAY = objects.column<3>()[objA];

            DynamicCollider2D* b = col.b;

            const double normalX = col.normalX;
            const double normalY = col.normalY;
            double penetrationDepth = col.penetrationDepth * resolvingStrength;
            if (b) {
                penetrationDepth *= 0.5;
            }
            double dx = normalX * penetrationDepth;
            double dy = normalY * penetrationDepth;
            if(b) {
                posAX += dx;
                posAY += dy;
                uint32_t objB = objectReg[b->objectID];
                objects.column<0>()[objB] -= dx;
                objects.column<1>()[objB] -= dy;
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
                double friction = ac.friction;
                double restitution = ac.restitution;
                double rx = (vx + nvx) * friction + nvx * restitution;
                double ry = (vy + nvy) * friction + nvy * restitution;
                prevPosAX = posAX - rx;
                prevPosAY = posAY - ry;
            }
        }
    }
    //time[7] = rdtsc();
    for(DynamicCollider2D& col : dynamicColliders) {
        const uint32_t loc = objectReg[col.objectID];
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

#endif
