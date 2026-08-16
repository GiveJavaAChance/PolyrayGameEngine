#ifndef COLLIDERSHAPE2D_H_INCLUDED
#define COLLIDERSHAPE2D_H_INCLUDED

#pragma once

#include <prvl.h>

#include <physics/2d/Physics2D.h>

namespace ColliderShape2D {
    struct AABB {
    };

    struct Circle {
    };

    inline bool collideAABB_AABB_2D(Collider2D& a, AABB* aData, Collider2D& b, AABB* bData, CollisionInfo2D& out) {
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

    inline bool collideCircle_Circle_2D(Collider2D& a, Circle* aData, Collider2D& b, Circle* bData, CollisionInfo2D& out) {
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

    inline bool collideCircle_AABB_2D(Collider2D& a, Circle* aData, Collider2D& b, AABB* bData, CollisionInfo2D& out) {
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

    inline static void registerBuiltinColliders(Physics2D* physicsSystem) {
        physicsSystem->registerCollision<AABB,   AABB,   collideAABB_AABB_2D>();
        physicsSystem->registerCollision<Circle, Circle, collideCircle_Circle_2D>();
        physicsSystem->registerCollision<Circle, AABB,   collideCircle_AABB_2D>();
    }
}

#endif