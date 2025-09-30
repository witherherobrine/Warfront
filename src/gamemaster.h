

#ifndef GAMEMASTER_H
#define GAMEMASTER_H


#include "raylib.h"
#include "raymath.h"

#include "entity.h"
#include "entitynode.h"
#include "pathmanager.h"


// ENTITY ANIM INDEX:

// - 0: DIE
// - 1: IDLE
// - 2: WALK

static EntityNode entNodes[2];



void gamemaster_setupNodes(Entity *entities, int size);
void gamemaster_moveTest(Entity *entities, int nodeId, Vector3 pos);


#endif