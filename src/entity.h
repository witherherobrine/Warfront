

#ifndef ENTITY_H
#define ENTITY_H


#include "raylib.h"
#include "raymath.h"



// ENTITY ANIM INDEX:

// - 0: DIE
// - 1: IDLE
// - 2: WALK

typedef struct Entity{
	int id;
	bool player;
	Vector3 position;
	float rotation;
	Vector3 velocity;
	Vector3 destination;
	BoundingBox aabb;
	Model model;
	bool alive;
	unsigned int animFrame;
	unsigned int animIndex;
	ModelAnimation* anims;
} Entity;

Entity entity_createEntity();
void entity_DrawEntity(Entity e);
void entity_updateEntity(Entity* e);
void entity_moveEntity(Entity* e, Vector3 pos);
void entity_queryBox(Entity* e,Vector3 position);
void entity_setEntityTarget(Entity* e,Vector3 position);
static bool atPosition(Vector3 ePos, Vector3 v);
void entity_switchAnimation(Entity* e, int index);


#endif