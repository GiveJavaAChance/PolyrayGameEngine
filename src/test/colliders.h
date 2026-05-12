#ifndef COLLIDERS_H_INCLUDED
#define COLLIDERS_H_INCLUDED

#pragma once

#include <prvl.h>
#include <physics/3d/Physics3D.h>

namespace CollisionShape {
    struct AABB {
    };

    struct Sphere {
    };

    struct Bean {
        dvec3 deltaA;
        dvec3 deltaB;
        double radius;
    };
}

inline static dvec3 project(dvec3 p, dvec3 a, dvec3 d) {
    return a + d * clamp(dot(p - a, d) / dot(d, d), 0.0, 1.0);
}

bool collideAABB_AABB(Collider3D& a, CollisionShape::AABB* aData, Collider3D& b, CollisionShape::AABB* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;
    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;
    double overlapX = halfAx + halfBx - abs(dx);
    double overlapY = halfAy + halfBy - abs(dy);
    double overlapZ = halfAz + halfBz - abs(dz);
    if (overlapX <= 0.0 || overlapY <= 0.0 || overlapZ <= 0.0) {
        return false;
    }
    if(overlapX < overlapY && overlapX < overlapZ) {
        out.collisionNormalX = (dx < 0.0) ? -1.0 : 1.0;
        out.collisionNormalY = 0.0;
        out.collisionNormalZ = 0.0;
        out.penetrationDepth = overlapX;
    } else if(overlapY < overlapZ) {
        out.collisionNormalX = 0.0;
        out.collisionNormalY = (dy < 0.0) ? -1.0 : 1.0;
        out.collisionNormalZ = 0.0;
        out.penetrationDepth = overlapY;
    } else {
        out.collisionNormalX = 0.0;
        out.collisionNormalY = 0.0;
        out.collisionNormalZ = (dz < 0.0) ? -1.0 : 1.0;
        out.penetrationDepth = overlapZ;
    }
    return true;
}

bool collideSphere_Sphere(Collider3D& a, CollisionShape::Sphere* aData, Collider3D& b, CollisionShape::Sphere* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;

    double ra = halfAx;
    double rb = halfBx;

    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;

    double dist2 = dx * dx + dy * dy + dz * dz;
    double r = ra + rb;
    if(dist2 > r * r) {
        return false;
    }
    double dist = sqrt(dist2);
    double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

    out.collisionNormalX = dx * invDist;
    out.collisionNormalY = dy * invDist;
    out.collisionNormalZ = dz * invDist;
    out.penetrationDepth = r - dist;
    return true;
}

bool collideSphere_AABB(Collider3D& a, CollisionShape::Sphere* aData, Collider3D& b, CollisionShape::AABB* bData, CollisionInfo3D& out) {
    double halfAx = a.sizeX * 0.5;
    double halfAy = a.sizeY * 0.5;
    double halfAz = a.sizeZ * 0.5;
    double halfBx = b.sizeX * 0.5;
    double halfBy = b.sizeY * 0.5;
    double halfBz = b.sizeZ * 0.5;
    double ax = a.posX + halfAx;
    double ay = a.posY + halfAy;
    double az = a.posZ + halfAz;
    double bx = b.posX + halfBx;
    double by = b.posY + halfBy;
    double bz = b.posZ + halfBz;

    double r = halfAx;

    double cx = max(bx - halfBx, min(ax, bx + halfBx));
    double cy = max(by - halfBy, min(ay, by + halfBy));
    double cz = max(bz - halfBz, min(az, bz + halfBz));

    double dx = ax - cx;
    double dy = ay - cy;
    double dz = az - cz;

    double dist2 = dx * dx + dy * dy + dz * dz;

    if (dist2 > r * r) {
        return false;
    }
    double dist = sqrt(dist2);
    double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

    out.collisionNormalX = dx * invDist;
    out.collisionNormalY = dy * invDist;
    out.collisionNormalZ = dz * invDist;
    out.penetrationDepth = r - dist;

    return true;
}

bool collideBean_Bean(Collider3D& a, CollisionShape::Bean* aData, Collider3D& b, CollisionShape::Bean* bData, CollisionInfo3D& out) {
    dvec3 a0 = prvl::dvec3(a.posX, a.posY, a.posZ) + aData->deltaA;
    dvec3 d0 = aData->deltaB - aData->deltaA;
    dvec3 a1 = prvl::dvec3(b.posX, b.posY, b.posZ) + bData->deltaA;
    dvec3 b1 = prvl::dvec3(b.posX, b.posY, b.posZ) + bData->deltaB;
    dvec3 d1 = bData->deltaB - bData->deltaA;

    double minDist = aData->radius + bData->radius;
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
    out.collisionNormalX = D.x * invDist;
    out.collisionNormalY = D.y * invDist;
    out.collisionNormalZ = D.z * invDist;
    out.penetrationDepth = penetration;
    return true;
}

bool collideBean_AABB(Collider3D& a, CollisionShape::Bean* aData, Collider3D& b, CollisionShape::AABB* bData, CollisionInfo3D& out) {
    dvec3 min = prvl::dvec3(b.posX, b.posY, b.posZ);
    dvec3 max = min + prvl::dvec3(b.sizeX, b.sizeY, b.sizeZ);
    dvec3 corners[]{
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z}
    };
    double minDistSq = 1000000000.0;
    dvec3 nearest;
    dvec3 lineA = prvl::dvec3(a.posX, a.posY, a.posZ) + aData->deltaA;
    dvec3 lineD = aData->deltaB - aData->deltaA;
    for (uint32_t i = 0u; i < 8u; i++) {
        dvec3& corner = corners[i];
        dvec3 proj = project(corner, lineA, lineD);
        dvec3 d = proj - corner;
        double distSq = dot(d, d);
        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = d;
        }
    }
    if (minDistSq < aData->radius * aData->radius) {
        double minDist = sqrt(minDistSq);
        double invDist = (minDist > 1e-12) ? 1.0 / minDist : 0.0;
        out.collisionNormalX = nearest.x * invDist;
        out.collisionNormalY = nearest.y * invDist;
        out.collisionNormalZ = nearest.z * invDist;
        out.penetrationDepth = aData->radius - minDist;
        return true;
    }
    dvec3 facePoint = clamp(lineA, min, max);

    dvec3 projOnLine = project(facePoint, lineA, lineD);
    dvec3 delta = projOnLine - facePoint;
    double distSideSq = dot(delta, delta);
    if (distSideSq > aData->radius * aData->radius) {
        return false;
    }
    double distSide = sqrt(distSideSq);
    double invDist = (distSide > 1e-12) ? 1.0 / distSide : 0.0;
    out.collisionNormalX = delta.x * invDist;
    out.collisionNormalY = delta.y * invDist;
    out.collisionNormalZ = delta.z * invDist;
    out.penetrationDepth = aData->radius - distSide;
    return true;
}

bool collideBean_Sphere(Collider3D& a, CollisionShape::Bean* aData, Collider3D& b, CollisionShape::Sphere* bData, CollisionInfo3D& out) {
    dvec3 lineA = prvl::dvec3(a.posX, a.posY, a.posZ) + aData->deltaA;
    dvec3 lineD = aData->deltaB - aData->deltaA;
    double rb = b.sizeX * 0.5;
    dvec3 pos = prvl::dvec3(b.posX, b.posY, b.posZ) + rb;
    dvec3 d = pos - project(pos, lineA, lineD);

    double dist2 = dot(d, d);
    double r = aData->radius + rb;
    if(dist2 > r * r) {
        return false;
    }
    double dist = sqrt(dist2);
    double invDist = (dist > 1e-12) ? 1.0 / dist : 0.0;

    out.collisionNormalX = d.x * invDist;
    out.collisionNormalY = d.y * invDist;
    out.collisionNormalZ = d.z * invDist;
    out.penetrationDepth = r - dist;
    return true;
}

static void registerAllColliders(Physics3D* physicsSystem) {
    using namespace CollisionShape;
    physicsSystem->registerCollision<AABB,   AABB,   collideAABB_AABB>();
    physicsSystem->registerCollision<Sphere, Sphere, collideSphere_Sphere>();
    physicsSystem->registerCollision<Sphere, AABB,   collideSphere_AABB>();
    physicsSystem->registerCollision<Bean,   Bean,   collideBean_Bean>();
    physicsSystem->registerCollision<Bean,   AABB,   collideBean_AABB>();
    physicsSystem->registerCollision<Bean,   Sphere, collideBean_Sphere>();
}

#endif
