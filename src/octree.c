

#include "octree.h"

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
    // Initialize dynamic array for objects (add checks here too)
    dyanmicarray_init(&newNode->objects, OCTREE_CAPACITY);
    if (newNode->objects.data == NULL && OCTREE_CAPACITY > 0) { // Check if init failed to allocate data
        TraceLog(LOG_FATAL, "OCTREE_CREATE_NODE_ERROR: dyanmicarray_init failed for new node's objects!");
        free(newNode); // Clean up partially allocated node
        return NULL;
    }

    newNode->depth = depth; // THIS IS THE LINE WE ARE SUSPECTING
    
    // printf("OCT_CREATE_NODE_DEBUG: Created node %p (Depth: %d). Bounds: min={%f,%f,%f}, max={%f,%f,%f}\n",
           // (void*)newNode, newNode->depth, bounds.min.x, bounds.min.y, bounds.min.z,
           // bounds.max.x, bounds.max.y, bounds.max.z);

    return newNode;
}

// Function to free an octree node and its descendants
// void octree_freeNode(OctreeNode *node) {
    // if (node) {
        // for (int i = 0; i < 8; i++) {
            // octree_freeNode(node->children[i]);
        // }
        // dyanmicarray_free(&node->objects);
        // free(node);
    // }
// }

// Helper function to determine which of the 8 octants a point lies in
int octree_getOctantIndex(BoundingBox bounds, Vector3 position) {
    Vector3 center = {
        (bounds.min.x + bounds.max.x) / 2.0f,
        (bounds.min.y + bounds.max.y) / 2.0f,
        (bounds.min.z + bounds.max.z) / 2.0f
    };
    int index = 0;
    if (position.x > center.x) index |= 1;
    if (position.y > center.y) index |= 2;
    if (position.z > center.z) index |= 4;
    return index;
}

// Function to insert a WorldObject into the octree
void octree_insert(OctreeNode *node, WorldObject *object) {


    // Initial NULL checks (should have been caught by previous checks, but good to be safe)
    if (node == NULL) {
        TraceLog(LOG_FATAL, "OCT_INSERT_ERROR: NULL node passed to octree_insert!");
        exit(1);
    }
    if (object == NULL) {
        TraceLog(LOG_FATAL, "OCT_INSERT_ERROR: NULL object passed to octree_insert!");
        exit(1);
    }
		   
		   
    // Also, check for already collapsed bounds before even calling subdivide
    if (fabsf(node->bounds.min.x - node->bounds.max.x) < FLT_EPSILON &&
        fabsf(node->bounds.min.y - node->bounds.max.y) < FLT_EPSILON &&
        fabsf(node->bounds.min.z - node->bounds.max.z) < FLT_EPSILON) {
        TraceLog(LOG_FATAL, "OCTREE_INSERT_FATAL: Attempting to subdivide a zero-extent node! Node %p, Depth %d", (void*)node, node->depth);
        exit(1);
    }

		   
    // Sanity check on object AABB (add this from previous suggestions if not already)
    // This is CRUCIAL if CheckCollisionBoxes can't handle bad AABBs gracefully.
    if (isnan(object->aabb.min.x) || isinf(object->aabb.min.x) ||
        isnan(object->aabb.min.y) || isinf(object->aabb.min.y) ||
        isnan(object->aabb.min.z) || isinf(object->aabb.min.z) ||
        isnan(object->aabb.max.x) || isinf(object->aabb.max.x) ||
        isnan(object->aabb.max.y) || isinf(object->aabb.max.y) ||
        isnan(object->aabb.max.z) || isinf(object->aabb.max.z)) {
        TraceLog(LOG_FATAL, "OCT_INSERT_ERROR: WorldObject %d has NaN/Inf AABB values!", object->id);
        exit(1);
    }

    // Sanity check on node depth (moved here for clarity with the fix)
    if (node->depth > OCTREE_MAX_DEPTH) {
        // This case indicates we've gone beyond the intended max depth.
        // It shouldn't happen if the logic below is correct.
        // If it does, it means an object is still being pushed deeper
        // after it should have been stopped.
        TraceLog(LOG_FATAL, "OCT_INSERT_FATAL: Node depth (%d) exceeds OCTREE_MAX_DEPTH (%d)! Object %d. Node %p",
                 node->depth, OCTREE_MAX_DEPTH, object->id, (void*)node);
        exit(1);
    }
  

    // 1. Check if the object's AABB is completely contained within the node's bounds
    if (!CheckCollisionBoxes(node->bounds, object->aabb)) {
        return; // Object is outside this node's bounds
    }
    
    // IMPORTANT: Redesigned logic for leaf vs. subdivide
    // First, check if we've reached max depth. If so, ALWAYS add to this node.
    if (node->depth >= OCTREE_MAX_DEPTH) {
        dyanmicarray_add(&node->objects, object);
        return;
    }

    // Now, if NOT at max depth, proceed with normal capacity check and subdivision
    // 2. If the current node is a leaf node (has no children) and hasn't reached capacity
    if (node->children[0] == NULL && node->objects.count < OCTREE_CAPACITY) {
        dyanmicarray_add(&node->objects, object);
        return;
    }

    // 3. If the current node is a leaf node (has no children) AND has reached capacity (since we already handled max depth above), subdivide it
    if (node->children[0] == NULL) { // This condition implicitly means node->objects.count >= OCTREE_CAPACITY
        
        // ... (Your OCT_INSERT_SUBDIVIDE_TRIGGER print and zero-extent check here) ...

        octree_subdivide(node);
    }

		// 4. Determine which child octant the object belongs to (based on its AABB's center)
    Vector3 center = {
        (object->aabb.min.x + object->aabb.max.x) / 2.0f,
        (object->aabb.min.y + object->aabb.max.y) / 2.0f,
        (object->aabb.min.z + object->aabb.max.z) / 2.0f
    };
    
    // Check for NaN/Inf in center (if AABB was fine, but division resulted in bad values)
    if (isnan(center.x) || isinf(center.x) ||
        isnan(center.y) || isinf(center.y) ||
        isnan(center.z) || isinf(center.z)) {
        TraceLog(LOG_FATAL, "OCT_INSERT_ERROR: Object %d center is NaN/Inf!", object->id);
        exit(1);
    }


    int index = octree_getOctantIndex(node->bounds, center);

    // 5. Recursively insert the object into the appropriate child node
    octree_insert(node->children[index], object);
}



// Function to subdivide an octree node into 8 children
void octree_subdivide(OctreeNode *node) {
    Vector3 min = node->bounds.min;
    Vector3 max = node->bounds.max;
    Vector3 center = {
        (min.x + max.x) / 2.0f,
        (min.y + max.y) / 2.0f,
        (min.z + max.z) / 2.0f
    };

    // Create the 8 child octants
    node->children[0] = octree_createNode((BoundingBox){{min.x, min.y, min.z}, {center.x, center.y, center.z}}, node->depth + 1); // 000
    node->children[1] = octree_createNode((BoundingBox){{center.x, min.y, min.z}, {max.x, center.y, center.z}}, node->depth + 1); // 100
    node->children[2] = octree_createNode((BoundingBox){{min.x, center.y, min.z}, {center.x, max.y, center.z}}, node->depth + 1); // 010
    node->children[3] = octree_createNode((BoundingBox){{center.x, center.y, min.z}, {max.x, max.y, center.z}}, node->depth + 1); // 110
    node->children[4] = octree_createNode((BoundingBox){{min.x, min.y, center.z}, {center.x, center.y, max.z}}, node->depth + 1); // 001
    node->children[5] = octree_createNode((BoundingBox){{center.x, min.y, center.z}, {max.x, center.y, max.z}}, node->depth + 1); // 101
    node->children[6] = octree_createNode((BoundingBox){{min.x, center.y, center.z}, {center.x, max.y, max.z}}, node->depth + 1); // 011
    node->children[7] = octree_createNode((BoundingBox){{center.x, center.y, center.z}, {max.x, max.y, max.z}}, node->depth + 1); // 111

    // Re-insert existing objects into the new child nodes
    for (int i = 0; i < node->objects.count; i++) {
        WorldObject *object = (WorldObject *)node->objects.data[i];
        Vector3 objCenter = {
            (object->aabb.min.x + object->aabb.max.x) / 2.0f,
            (object->aabb.min.y + object->aabb.max.y) / 2.0f,
            (object->aabb.min.z + object->aabb.max.z) / 2.0f
        };
        int index = octree_getOctantIndex(node->bounds, objCenter);
        octree_insert(node->children[index], object);
    }

    // Clear the objects array in the parent node (they are now in the children)
    dyanmicarray_resetCount(&node->objects);
}

#include "octree.h"
#include <stdio.h> // For printf
#include <string.h> // For memset and strlen (or just a loop for indent)

// Helper function for recursive traversal and printing
static void octree_debugPrintStructure(OctreeNode *node, int level) {
    if (node == NULL) {
        return; // Nothing to print for a NULL node
    }

    // Create indentation string
    char indent[100]; // Max 99 levels of indentation (adjust if needed)
    memset(indent, ' ', sizeof(indent)); // Fill with spaces
    if (level < sizeof(indent)) {
        indent[level * 2] = '\0'; // Two spaces per level for readability
    } else {
        indent[sizeof(indent) - 1] = '\0'; // Safety: Truncate if level is too high
    }


    // Print information about the current node
    // printf("%sNode: %p | Depth: %d | Objects: %d | Children: %s\n",
           // indent, (void*)node, node->depth, node->objects.count,
           // (node->children[0] != NULL ? "YES" : "NO"));

    // Print bounds for the current node (optional, can be very verbose)
    // printf("%s  Bounds: min={%.2f,%.2f,%.2f}, max={%.2f,%.2f,%.2f}\n",
    //        indent, node->bounds.min.x, node->bounds.min.y, node->bounds.min.z,
    //        node->bounds.max.x, node->bounds.max.y, node->bounds.max.z);

    // If this node has objects, print their IDs (optional, can be very verbose)
    // if (node->objects.count > 0) {
    //     printf("%s  Objects in this node: [", indent);
    //     for (int i = 0; i < node->objects.count; ++i) {
    //         WorldObject *obj = (WorldObject *)dyanmicarray_get(&node->objects, i);
    //         if (obj) {
    //             printf("%d%s", obj->id, (i < node->objects.count - 1 ? ", " : ""));
    //         } else {
    //             printf("NULL%s", (i < node->objects.count - 1 ? ", " : ""));
    //         }
    //     }
    //     printf("]\n");
    // }


    // Recursively call for children if they exist
    if (node->children[0] != NULL) {
        for (int i = 0; i < 8; i++) {
            // Only print a line for the child if it's not NULL
            if (node->children[i] != NULL) {
                // printf("%s  Child %d:\n", indent, i);
                octree_debugPrintStructure(node->children[i], level + 1);
            }
        }
    }
}

// Public entry point function
void octree_debugPrintOutput(OctreeNode *root) {
    printf("\n--- Octree Structure Debug Output ---\n");
    if (root == NULL) {
        // printf("Octree root is NULL. Tree is empty.\n");
        return;
    }
    octree_debugPrintStructure(root, 0); // Start at level 0
    // printf("--- End Octree Structure Debug Output ---\n\n");
}










