

#include "octree.h"



#include <raylib.h>
#include <stdlib.h> 
#include <stdbool.h>
#include "dynamicarray.h"

// Define the maximum number of objects a leaf node can hold before subdivision
#define OCTREE_CAPACITY 8

// Define the maximum depth of the octree to prevent infinite subdivision
#define OCTREE_MAX_DEPTH 10

typedef struct OctreeNode {
    BoundingBox bounds;       // The spatial bounds of this node
    struct OctreeNode *children[8]; // Pointers to the 8 child octants
    DynamicArray objects;     // List of WorldObject pointers contained in this node
    int depth;                // The depth of this node in the octree (0 for root)
} OctreeNode;

// Function to initialize an octree node
OctreeNode* octree_createNode(BoundingBox bounds, int depth) {
    OctreeNode *newNode = (OctreeNode *)malloc(sizeof(OctreeNode));
    if (!newNode) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for octree node");
        return NULL;
    }

    newNode->bounds = bounds;
    for (int i = 0; i < 8; i++) {
        newNode->children[i] = NULL;
    }
    dynamicArrayInit(&newNode->objects, OCTREE_CAPACITY); // Initialize with a starting capacity
    newNode->depth = depth;

    return newNode;
}

// Function to free an octree node and its descendants
void octree_freeNode(OctreeNode *node) {
    if (node) {
        for (int i = 0; i < 8; i++) {
            octree_freeNode(node->children[i]);
        }
        dynamicArrayFree(&node->objects);
        free(node);
    }
}