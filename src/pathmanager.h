


#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"


void path_manager_moveIndividual(Entity* e);


void path_manager_moveGroup(Entity *entities, int *idArr, size_t entCount, Vector3 position);

#endif