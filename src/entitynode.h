

#ifndef ENTITYNODE_H
#define ENTITYNODE_H

#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"


typedef struct EntityNode{
	int id;
	int parent;
	int* children;
	size_t childCount;
	
} EntityNode;



EntityNode* entitynode_createEntityNode(int id, int parent);
void entitynode_addChild(EntityNode* node, int childId);
void entitynode_freeEntityNode(EntityNode* node);
int* entitynode_getChildren(const EntityNode* node, size_t* numChildren);

#endif


