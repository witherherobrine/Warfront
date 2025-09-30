

#ifndef OCTREE_H
#define OCTREE_H


#include <raylib.h>
#include <stdlib.h> 
#include <stdbool.h>
#include "dynamicarray.h"
#include "worldobject.h"
#include <math.h>
#include <float.h>

// Define the maximum number of objects a leaf node can hold before subdivision
#define OCTREE_CAPACITY 2

// Define the maximum depth of the octree to prevent infinite subdivision
#define OCTREE_MAX_DEPTH 16

typedef struct OctreeNode {
    BoundingBox bounds;       // The spatial bounds of this node
    struct OctreeNode *children[8]; // Pointers to the 8 child octants
    DynamicArray objects;     // List of WorldObject pointers contained in this node
    int depth;                // The depth of this node in the octree (0 for root)
} OctreeNode;

OctreeNode* octree_createNode(BoundingBox bounds, int depth);
void octree_freeNode(OctreeNode *node);
int octree_getOctantIndex(BoundingBox bounds, Vector3 position);
void octree_insert(OctreeNode *node, WorldObject *object);
void octree_subdivide(OctreeNode *node);
void octree_subdivide_and_redistribute(OctreeNode *node);
void octree_create_children(OctreeNode *node);
bool CheckAABBContainsAABB(BoundingBox container, BoundingBox contained);
Vector3 GetAABBCenter(BoundingBox box);
void octree_prune(OctreeNode *node);
void octree_debugPrintOutput(OctreeNode *root);
void octree_debugDraw(OctreeNode *node);




#endif