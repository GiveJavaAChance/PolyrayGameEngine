#ifndef COLLIDERSHAPE2D_H_INCLUDED
#define COLLIDERSHAPE2D_H_INCLUDED

#pragma once

#include <prvl.h>

#include <physics/2d/Physics2D.h>

namespace ColliderShape2D {
    struct AABB {
        inline static bool collideAABB_AABB_2D(const AABB* aabbA, const AABB* aabbB, const Collider2D& a, const Collider2D& b, CollisionInfo2D& out) {
            double halfAx = a.sizeX * 0.5;
            double halfAy = a.sizeY * 0.5;
            double halfBx = b.sizeX * 0.5;
            double halfBy = b.sizeY * 0.5;
            double ax = a.posX + halfAx;
            double ay = a.posY + halfAy;
            double bx = b.posX + halfBx;
            double by = b.posY + halfBy;
            double dx = ax - bx;
            double dy = ay - by;
            double overlapX = halfAx + halfBx - abs(dx);
            double overlapY = halfAy + halfBy - abs(dy);
            if (overlapX <= 0.0 || overlapY <= 0.0) {
                return false;
            }
            if (overlapX < overlapY) {
                out.collisionNormalX = (dx < 0.0) ? -1.0 : 1.0;
                out.collisionNormalY = 0.0;
                out.penetrationDepth = overlapX;
            } else {
                out.collisionNormalX = 0.0;
                out.collisionNormalY = (dy < 0.0) ? -1.0 : 1.0;
                out.penetrationDepth = overlapY;
            }
            return true;
        }

        inline static float intersectRay(const vec2& pos, const vec2& invDir, const vec2& min, const vec2& max) {
            constexpr uint32_t dim = 2u;
            float tMin = NEGATIVE_INFINITY;
            float tMax = POSITIVE_INFINITY;
            for (uint32_t i = 0u; i < dim; i++) {
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

        inline static RayHit2D raycast(const vec2& pos, const vec2& invDir, const vec2& min, const vec2& max) {
            constexpr uint32_t dim = 2u;
            RayHit2D hit{};

            uint32_t face = 0u;
            float tMin = NEGATIVE_INFINITY;
            float tMax = POSITIVE_INFINITY;
            for (uint32_t i = 0u; i < dim; i++) {
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
            vec2 p = prvl::vec2(pos[0u], pos[1u]);
            vec2 id = prvl::vec2(invDir[0u], invDir[1u]);
            vec2 min = prvl::vec2(bounds[0u], bounds[1u]);
            vec2 max = prvl::vec2(bounds[3u], bounds[4u]);
            return intersectRay(p, id, min, max);
        }

        inline static void raycast(const AABB* __restrict__ aabb, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds, RayHit2D& hit) {
            vec2 p = prvl::vec2(pos[0u], pos[1u]);
            vec2 id = prvl::vec2(invDir[0u], invDir[1u]);
            vec2 min = prvl::vec2(bounds[0u], bounds[1u]);
            vec2 max = prvl::vec2(bounds[3u], bounds[4u]);
            hit = raycast(p, id, min, max);
        }
    };

    struct Circle {
        inline static bool collideCircle_Circle_2D(const Circle* circleA, const Circle* circleB, const Collider2D& a, const Collider2D& b, CollisionInfo2D& out) {
            double halfAx = a.sizeX * 0.5;
            double halfAy = a.sizeY * 0.5;
            double halfBx = b.sizeX * 0.5;
            double halfBy = b.sizeY * 0.5;
            double ax = a.posX + halfAx;
            double ay = a.posY + halfAy;
            double bx = b.posX + halfBx;
            double by = b.posY + halfBy;

            double ra = halfAx;
            double rb = halfBx;

            double dx = ax - bx;
            double dy = ay - by;

            double dist2 = dx * dx + dy * dy;
            double r = ra + rb;
            if (dist2 > r * r) {
                return false;
            }
            double dist = sqrt(dist2);
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

            out.collisionNormalX = dx * invDist;
            out.collisionNormalY = dy * invDist;
            out.penetrationDepth = r - dist;
            return true;
        }

        inline static bool collideCircle_AABB_2D(const Circle* circle, const AABB* aabb, const Collider2D& a, const Collider2D& b, CollisionInfo2D& out) {
            double halfAx = a.sizeX * 0.5;
            double halfAy = a.sizeY * 0.5;
            double halfBx = b.sizeX * 0.5;
            double halfBy = b.sizeY * 0.5;
            double ax = a.posX + halfAx;
            double ay = a.posY + halfAy;
            double bx = b.posX + halfBx;
            double by = b.posY + halfBy;

            double r = halfAx;

            double cx = max(bx - halfBx, min(ax, bx + halfBx));
            double cy = max(by - halfBy, min(ay, by + halfBy));

            double dx = ax - cx;
            double dy = ay - cy;

            double dist2 = dx * dx + dy * dy;

            if (dist2 > r * r) {
                return false;
            }
            double dist = sqrt(dist2);
            double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

            out.collisionNormalX = dx * invDist;
            out.collisionNormalY = dy * invDist;
            out.penetrationDepth = r - dist;

            return true;
        }

        inline static float intersectRay(const vec2& pos, const vec2& dir, const vec2& circlePos, float radius) {
            vec2 sr = pos - circlePos;
            float b = 2.0f * dot(dir, sr);
            float c = dot(sr, sr) - radius * radius;
            float s = b * b - 4.0f * c;
            if (s < 0.0f) {
                return -1.0f;
            }
            float t = (-b - sqrt(s)) * 0.5f;
            return t < 0.0f ? -1.0f : t;
        }

        inline static RayHit2D raycast(const vec2& pos, const vec2& dir, const vec2& circlePos, float radius) {
            RayHit2D hit{};
            vec2 sr = pos - circlePos;
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
            hit.normal = normalize(pos + tMin * dir - circlePos);
            hit.didHit = true;
            return hit;
        }

        inline static float intersectRay(const Circle* __restrict__ circle, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds) {
            constexpr uint32_t dim = 2u;
            vec2 p = prvl::vec2(pos[0u], pos[1u]);
            vec2 d = prvl::vec2(dir[0u], dir[1u]);
            vec2 spherePos = prvl::vec2(bounds[0u] + bounds[0u + dim], bounds[1u] + bounds[1u + dim]) * 0.5f;
            float radius = (bounds[dim] - bounds[0u]) * 0.5f;
            return intersectRay(p, d, spherePos, radius);
        }

        inline static void raycast(const Circle* __restrict__ circle, const float* __restrict__ pos, const float* __restrict__ dir, const float* __restrict__ invDir, const float* __restrict__ bounds, RayHit2D& hit) {
            constexpr uint32_t dim = 2u;
            vec2 p = prvl::vec2(pos[0u], pos[1u]);
            vec2 d = prvl::vec2(dir[0u], dir[1u]);
            vec2 spherePos = prvl::vec2(bounds[0u] + bounds[0u + dim], bounds[1u] + bounds[1u + dim]) * 0.5f;
            float radius = (bounds[dim] - bounds[0u]) * 0.5f;
            hit = raycast(p, d, spherePos, radius);
        }
    };

    inline static void registerBuiltinColliders() {
        ColliderShape2DRegistry::registerColliderShape<AABB>();
        ColliderShape2DRegistry::registerCollisionFunction<AABB, AABB, AABB::collideAABB_AABB_2D>();

        ColliderShape2DRegistry::registerColliderShape<Circle>();
        ColliderShape2DRegistry::registerCollisionFunction<Circle, Circle, Circle::collideCircle_Circle_2D>();
        ColliderShape2DRegistry::registerCollisionFunction<Circle, AABB, Circle::collideCircle_AABB_2D>();
    }
}

#endif