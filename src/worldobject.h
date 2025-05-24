

#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include <math.h>

#include "dynamicarray.h"

typedef enum {
    OBJECT_ENTITY,
    OBJECT_BULLET,
    OBJECT_TERRAIN,
    OBJECT_STRUCTURE
} ObjectType;

typedef struct WorldObject { 
    int id;
    ObjectType type;
    BoundingBox aabb;
    void *data;
} WorldObject;

extern DynamicArray worldObjects;

void worldobject_generateEntity(bool isPlayer, Vector3 position);
void worldobject_generateBullet();
void worldobject_updateAABB_entity(BoundingBox* box, Entity* e);
void worldobject_updateAABB_bullet();

#endif



// void checkCollisions(DynamicArray *worldObjects) {
    // for (int i = 0; i < worldObjects->count; i++) {
        // WorldObject *wo1 = (WorldObject *)worldObjects->data[i];

        // for (int j = i + 1; j < worldObjects->count; j++) { // Start j from i+1 to avoid duplicates and self-collision
            // WorldObject *wo2 = (WorldObject *)worldObjects->data[j];

            // if (wo1->id == wo2->id) continue;

            // Quick check for inactive objects
            // bool wo1_active = true;
            // if (wo1->type == OBJECT_BULLET) wo1_active = ((BulletData *)wo1->data)->enabled;
            // else if (wo1->type == OBJECT_ENTITY) wo1_active = ((EntityData *)wo1->data)->alive;
            // bool wo2_active = true;
            // if (wo2->type == OBJECT_BULLET) wo2_active = ((BulletData *)wo2->data)->enabled;
            // else if (wo2->type == OBJECT_ENTITY) wo2_active = ((EntityData *)wo2->data)->alive;

            // if (!wo1_active || !wo2_active) continue;

            // Optional: Quick distance check
            // Vector3 center1 = GetBoundingBoxCenter(wo1->aabb);
            // Vector3 center2 = GetBoundingBoxCenter(wo2->aabb);
            // float maxDistSq = (GetLength(GetBoundingBoxSize(wo1->aabb)) + GetLength(GetBoundingBoxSize(wo2->aabb))) / 2.0f;
            // maxDistSq *= maxDistSq * 1.1f; // Add a small safety factor
            // if (Vector3DistanceSqr(center1, center2) > maxDistSq) continue;

            // if (CheckCollisionBoxes(wo1->aabb, wo2->aabb)) {
                // Broad-phase collision, now handle narrow-phase based on types
                // if (wo1->type == OBJECT_BULLET) {
                    // BulletData *bullet = (BulletData *)wo1->data;
                    // if (wo2->type == OBJECT_ENTITY) {
                        // EntityData *entity = (EntityData *)wo2->data;
                        // RayCollision hit = GetRayCollisionBox(bullet->ray, entity->aabb);
                        // if (hit.hit && hit.distance <= bullet->velocity) {
                            // entity->alive = false;
                            // bullet->enabled = false;
                            // bullet->velocity = 0.0f;
                            // continue; // Bullet hit, no need to check further collisions for this bullet
                        // }
                    // }
                    // ... handle bullet-terrain, bullet-structure
                // } else if (wo1->type == OBJECT_ENTITY) {
                    // EntityData *entity1 = (EntityData *)wo1->data;
                    // if (wo2->type == OBJECT_ENTITY) {
                        // EntityData *entity2 = (EntityData *)wo2->data;
                        // Handle entity-entity collision (e.g., movement resolution)
                    // }
                    // ... handle entity-structure
                // }
                // ... handle other combinations
            // }
        // }
    // }
// }