#ifndef COLLIDERSHAPE3D_H_INCLUDED
#define COLLIDERSHAPE3D_H_INCLUDED

#pragma once

#include <prvl.h>

#include <physics/3d/Physics3D.h>

namespace ColliderShape3D {
    struct AABB {
        inline static bool collideAABB_AABB_3D(const AABB* aabbA, const AABB* aabbB, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            dvec3 halfA = a.size * 0.5;
            dvec3 halfB = b.size * 0.5;
            dvec3 ca = a.pos + halfA;
            dvec3 cb = b.pos + halfB;
            dvec3 d = ca - cb;
            dvec3 overlap = halfA + halfB - abs(d);
            if (overlap.x <= 0.0 || overlap.y <= 0.0 || overlap.z <= 0.0) {
                return false;
            }
            if (overlap.x < overlap.y && overlap.x < overlap.z) {
                out.collisionNormal.x = (d.x < 0.0) ? -1.0 : 1.0;
                out.collisionNormal.y = 0.0;
                out.collisionNormal.z = 0.0;
                out.penetrationDepth = overlap.x;
            } else if (overlap.y < overlap.z) {
                out.collisionNormal.x = 0.0;
                out.collisionNormal.y = (d.y < 0.0) ? -1.0 : 1.0;
                out.collisionNormal.z = 0.0;
                out.penetrationDepth = overlap.y;
            } else {
                out.collisionNormal.x = 0.0;
                out.collisionNormal.y = 0.0;
                out.collisionNormal.z = (d.z < 0.0) ? -1.0 : 1.0;
                out.penetrationDepth = overlap.z;
            }
            return true;
        }

        inline static float intersectRay(const vec3& pos, const vec3& invDir, const vec3& min, const vec3& max) {
            constexpr uint32_t dim = 3u;
            float tMin = NEGATIVE_INFINITY;
            float tMax = POSITIVE_INFINITY;
            for (uint32_t i = 0u; i < dim; i++) {
                if (std::isinf(invDir[i])) {
                    if (pos[i] < min[i] || pos[i] > max[i]) {
                        return -1.0f;
                    }
                    continue;
                }
                float t0 = (min[i] - pos[i]) * invDir[i];
                float t1 = (max[i] - pos[i]) * invDir[i];
                if (invDir[i] < 0.0f) {
                    float tmp = t0;
                    t0 = t1;
                    t1 = tmp;
                }
                tMin = t0 > tMin ? t0 : tMin;
                tMax = t1 < tMax ? t1 : tMax;
                if (tMax < tMin) {
                    return -1.0f;
                }
            }
            return tMin < 0.0f ? -1.0f : tMin;
        }

        inline static RayHit3D raycast(const vec3& pos, const vec3& invDir, const vec3& min, const vec3& max) {
            constexpr uint32_t dim = 3u;
            RayHit3D hit{};

            uint32_t face = 0u;
            float tMin = NEGATIVE_INFINITY;
            float tMax = POSITIVE_INFINITY;
            for (uint32_t i = 0u; i < dim; i++) {
                if (std::isinf(invDir[i])) {
                    if (pos[i] < min[i] || pos[i] > max[i]) {
                        return hit;
                    }
                    continue;
                }
                float t0 = (min[i] - pos[i]) * invDir[i];
                float t1 = (max[i] - pos[i]) * invDir[i];
                if (invDir[i] < 0.0f) {
                    float tmp = t0;
                    t0 = t1;
                    t1 = tmp;
                }
                if (t0 > tMin) {
                    tMin = t0;
                    face = i;
                }
                tMax = t1 < tMax ? t1 : tMax;
                if (tMax < tMin) {
                    return hit;
                }
            }
            if (tMin < 0.0f) {
                return hit;
            }
            hit.distance = tMin;
            hit.depth = tMax - tMin;
            hit.normal[face] = -sign(invDir[face]);
            hit.didHit = true;
            return hit;
        }

        inline static float intersectRay(const AABB* __restrict__ aabb, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds) {
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 id = prvl::vec3(invDir[0u], invDir[1u], invDir[2u]);
            vec3 min = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]);
            vec3 max = prvl::vec3(bounds[3u], bounds[4u], bounds[5u]);
            return intersectRay(p, id, min, max);
        }

        inline static void raycast(const AABB* __restrict__ aabb, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds, RayHit3D& hit) {
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 id = prvl::vec3(invDir[0u], invDir[1u], invDir[2u]);
            vec3 min = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]);
            vec3 max = prvl::vec3(bounds[3u], bounds[4u], bounds[5u]);
            hit = raycast(p, id, min, max);
        }
    };

    struct Sphere {
        inline static bool collideSphere_Sphere_3D(const Sphere* sphereA, const Sphere* sphereB, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            dvec3 halfA = a.size * 0.5;
            dvec3 halfB = b.size * 0.5;
            dvec3 ca = a.pos + halfA;
            dvec3 cb = b.pos + halfB;

            double ra = halfA.x;
            double rb = halfB.x;

            dvec3 d = ca - cb;

            double dist2 = dot(d, d);
            double r = ra + rb;
            if (dist2 > r * r) {
                return false;
            }
            double dist = sqrt(dist2);
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

            out.collisionNormal = d * invDist;
            out.penetrationDepth = r - dist;
            return true;
        }

        inline static bool collideSphere_AABB_3D(const Sphere* sphere, const AABB* aabb, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            dvec3 halfA = a.size * 0.5;
            dvec3 halfB = b.size * 0.5;
            dvec3 ca = a.pos + halfA;
            dvec3 cb = b.pos + halfB;
            dvec3 d = ca - cb;
            dvec3 overlap = halfA + halfB - abs(d);
            double minOverlap = min(min(overlap.x, overlap.y), overlap.z);
            if (minOverlap > 0.0) {
                if (overlap.x < overlap.y && overlap.x < overlap.z) {
                    out.collisionNormal.x = sign(d.x) * overlap.x;
                    out.collisionNormal.y = 0.0;
                    out.collisionNormal.z = 0.0;
                } else if (overlap.y < overlap.z) {
                    out.collisionNormal.x = 0.0;
                    out.collisionNormal.y = sign(d.y) * overlap.y;
                    out.collisionNormal.z = 0.0;
                } else {
                    out.collisionNormal.x = 0.0;
                    out.collisionNormal.y = 0.0;
                    out.collisionNormal.z = sign(d.z) * overlap.z;
                }
                out.penetrationDepth = minOverlap;
                return true;
            }

            double r = halfA.x;

            d = ca - max(b.pos, min(ca, cb + halfB));

            double dist2 = dot(d, d);
            if (dist2 > r * r) {
                return false;
            }
            double dist = sqrt(dist2);
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

            out.collisionNormal = d * invDist;
            out.penetrationDepth = r - dist;

            return true;
        }

        inline static float intersectRay(const vec3& pos, const vec3& dir, const vec3& spherePos, float radius) {
            vec3 sr = pos - spherePos;
            float b = 2.0f * dot(dir, sr);
            float c = dot(sr, sr) - radius * radius;
            float s = b * b - 4.0f * c;
            if (s < 0.0f) {
                return -1.0f;
            }
            float t = (-b - sqrt(s)) * 0.5f;
            return t < 0.0f ? -1.0f : t;
        }

        inline static RayHit3D raycast(const vec3& pos, const vec3& dir, const vec3& spherePos, float radius) {
            RayHit3D hit{};
            vec3 sr = pos - spherePos;
            float b = 2.0f * dot(dir, sr);
            float c = dot(sr, sr) - radius * radius;
            float s = b * b - 4.0f * c;
            if (s < 0.0f) {
                return hit;
            }
            float tMin = (-b - sqrt(s)) * 0.5f;
            if (tMin < 0.0f) {
                return hit;
            }
            float tMax = (-b + sqrt(s)) * 0.5f;
            hit.distance = tMin;
            hit.depth = tMax - tMin;
            hit.normal = normalize(pos + tMin * dir - spherePos);
            hit.didHit = true;
            return hit;
        }

        inline static float intersectRay(const Sphere* __restrict__ sphere, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds) {
            constexpr uint32_t dim = 3u;
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 d = prvl::vec3(dir[0u], dir[1u], dir[2u]);
            vec3 spherePos = prvl::vec3((bounds[0u] + bounds[0u + dim]) * 0.5f, (bounds[1u] + bounds[1u + dim]) * 0.5f, (bounds[2u] + bounds[2u + dim]) * 0.5f);
            float radius = (bounds[dim] - bounds[0u]) * 0.5f;
            return intersectRay(p, d, spherePos, radius);
        }

        inline static void raycast(const Sphere* __restrict__ sphere, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds, RayHit3D& hit) {
            constexpr uint32_t dim = 3u;
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 d = prvl::vec3(dir[0u], dir[1u], dir[2u]);
            vec3 spherePos = prvl::vec3((bounds[0u] + bounds[0u + dim]) * 0.5f, (bounds[1u] + bounds[1u + dim]) * 0.5f, (bounds[2u] + bounds[2u + dim]) * 0.5f);
            float radius = (bounds[dim] - bounds[0u]) * 0.5f;
            hit = raycast(p, d, spherePos, radius);
        }
    };

    struct Bean {
        dvec3 deltaA;
        dvec3 deltaB;
        double radius;

        inline static dvec3 project(dvec3 p, dvec3 a, dvec3 d) {
            return a + d * clamp(dot(p - a, d) / dot(d, d), 0.0, 1.0);
        }

        inline static bool collideBean_Bean_3D(const Bean* beanA, const Bean* beanB, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            dvec3 a0 = a.pos + beanA->deltaA;
            dvec3 d0 = beanA->deltaB - beanA->deltaA;
            dvec3 a1 = b.pos + beanB->deltaA;
            dvec3 b1 = b.pos + beanB->deltaB;
            dvec3 d1 = beanB->deltaB - beanB->deltaA;

            double minDist = beanA->radius + beanB->radius;
            dvec3 A0 = project(a1, a0, d0);
            dvec3 B0 = project(b1, a0, d0);
            dvec3 A1 = project(A0, a1, d1);
            dvec3 B1 = project(B0, a1, d1);
            dvec3 D0 = A0 - A1;
            dvec3 D1 = B0 - B1;
            double dist0 = length(D0);
            double dist1 = length(D1);

            double dist = dist0 < dist1 ? dist0 : dist1;
            if (dist > minDist) {
                return false;
            }
            double penetration = minDist - dist;
            dvec3 D = dist0 < dist1 ? D0 : D1;
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;
            out.collisionNormal = D * invDist;
            out.penetrationDepth = penetration;
            return true;
        }

        inline static bool collideBean_AABB_3D(const Bean* bean, const AABB* aabb, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            constexpr double EPSILON = 1e-12;

            dvec3 min = b.pos;
            dvec3 max = min + b.size;
            dvec3 A = a.pos + bean->deltaA;
            dvec3 B = a.pos + bean->deltaB;
            dvec3 D = bean->deltaB - bean->deltaA;

            double cuts[8u];
            uint32_t cutCount = 0u;
            cuts[cutCount++] = 0.0;
            cuts[cutCount++] = 1.0;

            dvec3 ad = abs(D);
            dvec3 mina = (min - A) / D;
            dvec3 maxa = (max - A) / D;
            if (ad.x > EPSILON) {
                if (mina.x > 0.0 && mina.x < 1.0) {
                    cuts[cutCount++] = mina.x;
                }
                if (maxa.x > 0.0 && maxa.x < 1.0) {
                    cuts[cutCount++] = maxa.x;
                }
            }
            if (ad.y > EPSILON) {
                if (mina.y > 0.0 && mina.y < 1.0) {
                    cuts[cutCount++] = mina.y;
                }
                if (maxa.y > 0.0 && maxa.y < 1.0) {
                    cuts[cutCount++] = maxa.y;
                }
            }
            if (ad.z > EPSILON) {
                if (mina.z > 0.0 && mina.z < 1.0) {
                    cuts[cutCount++] = mina.z;
                }
                if (maxa.z > 0.0 && maxa.z < 1.0) {
                    cuts[cutCount++] = maxa.z;
                }
            }

            for (uint32_t i = 1u; i < cutCount; i++) {
                double value = cuts[i];
                uint32_t j = i;
                while (j > 0u && cuts[j - 1u] > value) {
                    cuts[j] = cuts[j - 1u];
                    j--;
                }
                cuts[j] = value;
            }
            double minDistSq = bean->radius * bean->radius + 1.0;
            dvec3 bestSegmentPoint;

            dvec3 delta;

            for (uint32_t i = 0u; i + 1u < cutCount; i++) {
                double t0 = cuts[i];
                double t1 = cuts[i + 1u];

                double tm = (t0 + t1) * 0.5;
                dvec3 Pm = A + D * tm;

                bvec3 smin = Pm < min;
                bvec3 smax = Pm > max;

                bvec3 active = smin | smax;

                dvec3 C = select(min, select(max, prvl::dvec3(), smax), smin);

                dvec3 da = D * D;
                dvec3 db = D * (A - C);

                dvec3 qav = select(da, prvl::dvec3(0.0), active);
                dvec3 qbv = select(db, prvl::dvec3(0.0), active);
                double qa = qav.x + qav.y + qav.z;
                double qb = qbv.x + qbv.y + qbv.z;

                double t = qa > EPSILON ? clamp(-qb / qa, t0, t1) : t0;

                dvec3 p = A + D * t;
                dvec3 d = p - clamp(p, min, max);
                double distSq = dot(d, d);
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    bestSegmentPoint = p;
                    delta = d;
                }
            }
            dvec3 da = A - clamp(A, min, max);
            double distSqA = dot(da, da);
            if (distSqA < minDistSq) {
                minDistSq = distSqA;
                bestSegmentPoint = A;
                delta = da;
            }
            dvec3 db = B - clamp(B, min, max);
            double distSqB = dot(db, db);
            if (distSqB < minDistSq) {
                minDistSq = distSqB;
                bestSegmentPoint = B;
                delta = db;
            }
            if (minDistSq >= bean->radius * bean->radius) {
                return false;
            }
            double dist = sqrt(minDistSq);
            if (dist <= EPSILON) {
                dvec3 P = bestSegmentPoint;

                double left = P.x - min.x;
                double right = max.x - P.x;
                double bottom = P.y - min.y;
                double top = max.y - P.y;
                double back = P.z - min.z;
                double front = max.z - P.z;

                double penetration = left;
                dvec3 normal = prvl::dvec3(-1.0, 0.0, 0.0);
                if (right < penetration) {
                    penetration = right;
                    normal = prvl::dvec3(1.0, 0.0, 0.0);
                }
                if (bottom < penetration) {
                    penetration = bottom;
                    normal = prvl::dvec3(0.0, -1.0, 0.0);
                }
                if (top < penetration) {
                    penetration = top;
                    normal = prvl::dvec3(0.0, 1.0, 0.0);
                }
                if (back < penetration) {
                    penetration = back;
                    normal = prvl::dvec3(0.0, 0.0, -1.0);
                }
                if (front < penetration) {
                    penetration = front;
                    normal = prvl::dvec3(0.0, 0.0, 1.0);
                }
                out.collisionNormal = normal;
                out.penetrationDepth = bean->radius + penetration;
                return true;
            }
            double invDist = 1.0 / dist;
            out.collisionNormal = delta * invDist;
            out.penetrationDepth = bean->radius - dist;
            return true;
        }

        inline static bool collideBean_Sphere_3D(const Bean* bean, const Sphere* sphere, const Collider3D& a, const Collider3D& b, CollisionInfo3D& out) {
            dvec3 lineA = a.pos + bean->deltaA;
            dvec3 lineD = bean->deltaB - bean->deltaA;
            double rb = b.size.x * 0.5;
            dvec3 pos = b.pos + rb;
            dvec3 d = project(pos, lineA, lineD) - pos;

            double dist2 = dot(d, d);
            double r = bean->radius + rb;
            if (dist2 > r * r) {
                return false;
            }
            double dist = sqrt(dist2);
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

            out.collisionNormal = d * invDist;
            out.penetrationDepth = r - dist;
            return true;
        }

        inline static float intersectRay(const vec3& pos, const vec3& dir, const vec3& a, const vec3& b, float radius) {
            vec3 AB = b - a;
            vec3 AO = pos - a;

            float AB_dot_d = dot(AB, dir);
            float AB_dot_AO = dot(AB, AO);
            float AB_dot_AB = dot(AB, AB);

            float m = AB_dot_d / AB_dot_AB;
            float n = AB_dot_AO / AB_dot_AB;

            vec3 Q = dir - (AB * m);
            vec3 R = AO - (AB * n);

            float A = dot(Q, Q);
            float B = 2.0f * dot(Q, R);
            float C = dot(R, R) - (radius * radius);

            if (A == 0.0f) {
                float distA = Sphere::intersectRay(pos, dir, a, radius);
                float distB = Sphere::intersectRay(pos, dir, b, radius);

                if (distA < 0.0f && distB < 0.0f) {
                    return -1.0f;
                }
                if (distA < 0.0f) {
                    return distB;
                }
                if (distB < 0.0f) {
                    return distA;
                }
                return min(distA, distB);
            }

            float discriminant = B * B - 4.0f * A * C;
            if (discriminant < 0.0f) {
                return -1.0f;
            }

            float tMin = (-B - sqrt(discriminant)) / (2.0f * A);
            float tMax = (-B + sqrt(discriminant)) / (2.0f * A);
            if (tMin > tMax) {
                float tmp = tMin;
                tMin = tMax;
                tMax = tmp;
            }
            float u = tMin * m + n;
            if (u < 0.0f) {
                return Sphere::intersectRay(pos, dir, a, radius);
            } else if (u > 1.0f) {
                return Sphere::intersectRay(pos, dir, b, radius);
            } else {
                return tMin;
            }
        }

        inline static RayHit3D raycast(const vec3& pos, const vec3& dir, const vec3& a, const vec3& b, float radius) {
            RayHit3D hit{};

            vec3 AB = b - a;
            vec3 AO = pos - a;

            float AB_dot_d = dot(AB, dir);
            float AB_dot_AO = dot(AB, AO);
            float AB_dot_AB = dot(AB, AB);

            float m = AB_dot_d / AB_dot_AB;
            float n = AB_dot_AO / AB_dot_AB;

            vec3 Q = dir - (AB * m);
            vec3 R = AO - (AB * n);

            float A = dot(Q, Q);
            float B = 2.0f * dot(Q, R);
            float C = dot(R, R) - (radius * radius);

            float tMin;
            float tMax;

            RayHit3D aHit = Sphere::raycast(pos, dir, a, radius);
            RayHit3D bHit = Sphere::raycast(pos, dir, b, radius);

            float aHitMax = aHit.distance + aHit.depth;
            float bHitMax = bHit.distance + bHit.depth;

            if (A == 0.0f) {
                if (!aHit.didHit || !bHit.didHit) {
                    return hit;
                }
                if (aHit.distance < bHit.distance) {
                    tMin = aHit.distance;
                    hit.normal = aHit.normal;
                } else {
                    tMin = bHit.distance;
                    hit.normal = bHit.normal;
                }
                tMax = max(aHitMax, bHitMax);
                hit.distance = tMin;
                hit.depth = tMax - tMin;
                hit.didHit = true;
                return hit;
            }

            float discriminant = B * B - 4.0f * A * C;
            if (discriminant < 0.0f) {
                return hit;
            }

            tMin = (-B - sqrt(discriminant)) / (2.0f * A);
            tMax = (-B + sqrt(discriminant)) / (2.0f * A);
            if (tMin > tMax) {
                float tmp = tMin;
                tMin = tMax;
                tMax = tmp;
            }
            float u = tMin * m + n;
            if (u < 0.0f) {
                if (!aHit.didHit) {
                    return hit;
                }
                tMin = aHit.distance;
                hit.normal = aHit.normal;
            } else if (u > 1.0f) {
                if (!bHit.didHit) {
                    return hit;
                }
                tMin = bHit.distance;
                hit.normal = bHit.normal;
            } else {
                hit.normal = normalize(pos + tMin * dir - (a + u * AB));
            }
            u = tMax * m + n;
            if (u < 0.0f) {
                if (!aHit.didHit) {
                    return hit;
                }
                tMax = aHitMax;
            } else if (u > 1.0f) {
                if (!bHit.didHit) {
                    return hit;
                }
                tMax = bHitMax;
            }
            hit.distance = tMin;
            hit.depth = tMax - tMin;
            hit.didHit = true;
            return hit;
        }

        inline static float intersectRay(const Bean* __restrict__ bean, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds) {
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 d = prvl::vec3(dir[0u], dir[1u], dir[2u]);

            vec3 a = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]) + prvl::vec3(bean->deltaA);
            vec3 b = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]) + prvl::vec3(bean->deltaB);
            float radius = static_cast<float>(bean->radius);
            return intersectRay(p, d, a, b, radius);
        }

        inline static void raycast(const Bean* __restrict__ bean, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds, RayHit3D& hit) {
            vec3 p = prvl::vec3(pos[0u], pos[1u], pos[2u]);
            vec3 d = prvl::vec3(dir[0u], dir[1u], dir[2u]);

            vec3 a = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]) + prvl::vec3(bean->deltaA);
            vec3 b = prvl::vec3(bounds[0u], bounds[1u], bounds[2u]) + prvl::vec3(bean->deltaB);
            float radius = static_cast<float>(bean->radius);
            hit = raycast(p, d, a, b, radius);
        }
    };

    inline static void registerBuiltinColliders() {
        ColliderShape3DRegistry::registerColliderShape<AABB>();
        ColliderShape3DRegistry::registerCollisionFunction<AABB, AABB, AABB::collideAABB_AABB_3D>();

        ColliderShape3DRegistry::registerColliderShape<Sphere>();
        ColliderShape3DRegistry::registerCollisionFunction<Sphere, Sphere, Sphere::collideSphere_Sphere_3D>();
        ColliderShape3DRegistry::registerCollisionFunction<Sphere, AABB, Sphere::collideSphere_AABB_3D>();

        ColliderShape3DRegistry::registerColliderShape<Bean>();
        ColliderShape3DRegistry::registerCollisionFunction<Bean, Bean, Bean::collideBean_Bean_3D>();
        ColliderShape3DRegistry::registerCollisionFunction<Bean, AABB, Bean::collideBean_AABB_3D>();
        ColliderShape3DRegistry::registerCollisionFunction<Bean, Sphere, Bean::collideBean_Sphere_3D>();
    }
}

#endif