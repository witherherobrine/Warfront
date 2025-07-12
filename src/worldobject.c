

#include "worldobject.h"
#include "entity.h"
#include <stdlib.h>
#include "raylib.h"
#include "bullet.h"



DynamicArray worldObjects;




static float ENT_AABB_LENGTH = 0.5f;
static float ENT_AABB_WIDTH = 0.5f;
static float ENT_AABB_HEIGHT = 2.2f;


void worldobject_generateDR(){
	dyanmicarray_init(&worldObjects, 0);
}

void worldobject_generateEntity(bool isPlayer, Vector3 position) {
	
    // 1. Allocate memory for EntityData
    Entity *newEntityData = malloc(sizeof(Entity));
    if (newEntityData == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for EntityData");
        return; // Handle allocation failure
    }
	
	Model model = LoadModel("../res/models/person/disgiuy.m3d");
	
	model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("../res/textures/newtexx.png"); // Set map diffuse texture


    int animsCount = 2;
	ModelAnimation* anim = LoadModelAnimations("../res/models/person/disgiuy.m3d", &animsCount);

    // 2. Initialize EntityData
    newEntityData->id = 0;
    newEntityData->player = isPlayer;
    newEntityData->position = position;
    newEntityData->rotation = 0.0f;
    newEntityData->velocity = (Vector3){0,0,0};
    newEntityData->destination = position;
    newEntityData->model = model;
    newEntityData->alive = true;
    newEntityData->animFrame = 0;
    newEntityData->animIndex = 2;
    newEntityData->anims = anim;
	
    WorldObject *newWorldObject = malloc(sizeof(WorldObject));
    if (newWorldObject == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for WorldObject");
        free(newEntityData); // Clean up allocated EntityData
        return; // Handle allocation failure
    }

    // 4. Initialize WorldObject
    newWorldObject->id = newEntityData->id;
    newWorldObject->type = OBJECT_ENTITY;
    newWorldObject->aabb = GetModelBoundingBox(model);
    newWorldObject->data = newEntityData; // Point to the allocated EntityData
	newWorldObject->currentOctreeNode = NULL; // <--- ADD THIS LINE
    newWorldObject->aabb_prev = newWorldObject->aabb; // <--- Also initialize aabb_prev

	
	dyanmicarray_add(&worldObjects, newWorldObject);

    // 5. Add WorldObject to the global array
    // dynamicArrayAdd(&worldObjects, newWorldObject);

}



void worldobject_generateBullet(){
	
    Bullet *newBulletData = malloc(sizeof(Bullet));
    if (newBulletData == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for Bullet");
        return; 
    }
	
	newBulletData -> id = 2;
	newBulletData -> ray = (Ray){(Vector3){0,0,0},(Vector3){0,1,0}};
	newBulletData -> velocity = 4.0f;
	newBulletData -> enabled = true;
	newBulletData -> origin = (Vector3){0,0,0};
	
    WorldObject *newWorldObject = malloc(sizeof(WorldObject));
    if (newWorldObject == NULL) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for WorldObject");
        free(newBulletData);
        return;
    }

    // 4. Initialize WorldObject
    newWorldObject->id = 0;
    newWorldObject->type = OBJECT_BULLET;
    newWorldObject->aabb = (BoundingBox){1};
    newWorldObject->data = newBulletData; // Point to the allocated EntityData
	newWorldObject->currentOctreeNode = NULL; // <--- ADD THIS LINE
    newWorldObject->aabb_prev = newWorldObject->aabb; // <--- Also initialize aabb_prev

	
	dyanmicarray_add(&worldObjects, newWorldObject);
	
}


void worldobject_updateAABB_entity(BoundingBox* box, Entity* e){
	box->min = (Vector3){e->position.x-ENT_AABB_LENGTH,e->position.y-ENT_AABB_HEIGHT,e->position.z-ENT_AABB_WIDTH};
	box->max = (Vector3){e->position.x+ENT_AABB_LENGTH,e->position.y+ENT_AABB_HEIGHT,e->position.z+ENT_AABB_WIDTH};
}

// Function to calculate the AABB for a bullet
void worldobject_updateAABB_bullet(BoundingBox* box, Bullet *bullet) {
	
    // Calculate the end point of the bullet's path based on velocity and time.
    Vector3 endPoint = Vector3Add(bullet->ray.position, Vector3Scale(bullet->ray.direction, bullet->velocity));

    // Determine the minimum and maximum coordinates of the AABB.
    // We need to consider both the starting point (ray.position) and the end point.
    Vector3 min = {
        fminf(bullet->ray.position.x, endPoint.x) - 0.05f, // Add a small size for visibility/collision
        fminf(bullet->ray.position.y, endPoint.y) - 0.05f,
        fminf(bullet->ray.position.z, endPoint.z) - 0.05f
    };
    Vector3 max = {
        fmaxf(bullet->ray.position.x, endPoint.x) + 0.05f,
        fmaxf(bullet->ray.position.y, endPoint.y) + 0.05f,
        fmaxf(bullet->ray.position.z, endPoint.z) + 0.05f
    };

	box->min = min;
	box->max = max;
	
}

// void worldobject_updateAABB_bullet(WorldObject* bullet){}


