

#include "octree.h"
#include <stdio.h> // For printf
#include <string.h> // For memset and strlen (or just a loop for indent)

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
void octree_free(OctreeNode **node) {
    if (*node == NULL) return;
    // Recursively free children first
    for (int i = 0; i < 8; i++) {
        octree_free(&((*node)->children[i]));
    }
    dyanmicarray_free(&((*node)->objects)); // Free the dynamic array itself
    free(*node); // Free the node struct
    *node = NULL; // Set the pointer to NULL
}

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
// Function to insert a WorldObject into the octree
void octree_insert(OctreeNode *node, WorldObject *object) {
    // Initial NULL and sanity checks
    // ... (Your existing sanity checks for node, object, AABB, and zero-extent node) ...

    // 1. Check if the object's AABB intersects the current node's bounds.
    // If it doesn't even intersect, it cannot be placed here or in children.
    if (!CheckCollisionBoxes(node->bounds, object->aabb)) {
        TraceLog(LOG_WARNING, "OCTREE_INSERT: Object ID %d AABB is outside node %p (Depth: %d) bounds. Not inserted.",
                 object->id, (void*)node, node->depth);
        return;
    }

    // 2. If this node is at max depth OR it's a leaf node (no children yet)
    //    AND has space (not over capacity) - add the object directly to this node.
    //    This is the "final resting place" for objects at max depth or in sparse areas.
    if (node->depth >= OCTREE_MAX_DEPTH || node->children[0] == NULL) { // If it's a leaf node or at max depth
        dyanmicarray_add(&node->objects, object);
        object->currentOctreeNode = node; // <<< CRITICAL: Update currentOctreeNode
        TraceLog(LOG_DEBUG, "OCTREE_INSERT: Object ID %d added to leaf/max-depth node %p (Depth: %d). Objects: %d",
                 object->id, (void*)node, node->depth, node->objects.count);

        // If at max depth, we never subdivide further.
        // If it's a leaf node (and not max depth yet), check if we need to subdivide.
        if (node->depth < OCTREE_MAX_DEPTH && node->objects.count > OCTREE_CAPACITY) {
            octree_subdivide_and_redistribute(node); // Subdivide and move objects down
        }
        return; // Object has been placed
    }

    // 3. If this is an internal node (has children and not at max depth),
    //    try to push the object down to a child.
    int children_object_is_contained_in = 0;
    int target_child_index = -1;

    for (int i = 0; i < 8; i++) {
        // Check if the object's AABB is COMPLETELY CONTAINED within this child's bounds.
        // This is crucial for avoiding over-subdivision due to straddling.
        if (CheckAABBContainsAABB(node->children[i]->bounds, object->aabb)) {
            children_object_is_contained_in++;
            target_child_index = i; // Store index of the first (or only) child
        }
    }

    if (children_object_is_contained_in == 1) {
        // Object fits perfectly into exactly one child, so recurse down.
        TraceLog(LOG_DEBUG, "OCTREE_INSERT: Object ID %d fits child %d. Recursing.", object->id, target_child_index);
        octree_insert(node->children[target_child_index], object);
    } else {
        // Object straddles multiple children, or doesn't fit into any single child perfectly,
        // or fits into 0 children (though `CheckCollisionBoxes` should have caught this at start).
        // Store it directly in this current node (the parent).
        // This prevents unnecessary deep subdivision for straddling objects.
        dyanmicarray_add(&node->objects, object);
        object->currentOctreeNode = node; // <<< CRITICAL: Update currentOctreeNode
        TraceLog(LOG_DEBUG, "OCTREE_INSERT: Object ID %d straddles/multiple children or no fit. Added to node %p (Depth: %d). Objects: %d",
                 object->id, (void*)node, node->depth, node->objects.count);

        // NOTE: An internal node (with children already) doesn't typically re-subdivide.
        // It just holds the straddling objects. Its children will manage their own subdivisions.
        // So, no `if (node->objects.count > OCTREE_CAPACITY)` check here.
    }
}

// Function to perform subdivision and re-distribute objects
// This is called when a leaf node exceeds capacity and is not at max depth.
void octree_subdivide_and_redistribute(OctreeNode *node) {
    if (node == NULL || node->depth >= OCTREE_MAX_DEPTH || node->children[0] != NULL) {
        TraceLog(LOG_ERROR, "OCTREE_SUBDIVIDE_REDISTRIBUTE: Invalid call. Node %p (Depth: %d).", (void*)node, node->depth);
        return;
    }
    TraceLog(LOG_DEBUG, "OCTREE_SUBDIVIDE: Node %p (Depth: %d) subdividing and redistributing.", (void*)node, node->depth);

    // 1. Create the 8 child nodes
    octree_create_children(node); // This function sets node->children[i]

    // 2. Temporarily store objects, then clear parent's object list
    DynamicArray temp_objects;
    dyanmicarray_init(&temp_objects, node->objects.count);
    for (int i = 0; i < node->objects.count; i++) {
        dyanmicarray_add(&temp_objects, dyanmicarray_get(&node->objects, i));
    }
    dyanmicarray_clear(&node->objects); // Clear the parent's current object list

    // 3. Re-insert ALL objects from the temporary list into the new children (or back into this node if straddling)
    // IMPORTANT: Recursively call octree_insert on the current node 'node',
    // which will then correctly route the objects to children or keep them here.
    for (int i = 0; i < temp_objects.count; i++) {
        WorldObject* obj_to_redistribute = (WorldObject*)dyanmicarray_get(&temp_objects, i);
        // This recursive call to octree_insert on the *current node* 'node'
        // will handle placing the object correctly into a child or back into 'node' itself.
        octree_insert(node, obj_to_redistribute);
    }
    dyanmicarray_free(&temp_objects); // Free the temporary array
}


// Function to create and initialize 8 child nodes for subdivision
// This should be called by octree_subdivide when a node needs to split.
void octree_create_children(OctreeNode *node) {
    if (node == NULL || node->children[0] != NULL) {
        TraceLog(LOG_ERROR, "OCTREE_CREATE_CHILDREN: Invalid node for subdivision or already has children.");
        return;
    }

    Vector3 center = GetAABBCenter(node->bounds);
    Vector3 halfSize = (Vector3){
        (node->bounds.max.x - node->bounds.min.x) / 2.0f,
        (node->bounds.max.y - node->bounds.min.y) / 2.0f,
        (node->bounds.max.z - node->bounds.min.z) / 2.0f
    };

    // Define child bounds systematically
    // Iterate through X, Y, Z quadrants
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            for (int z = 0; z < 2; z++) {
                int i = x * 4 + y * 2 + z; // Index mapping (0-7)

                BoundingBox childBounds;
                childBounds.min.x = (x == 0) ? node->bounds.min.x : center.x;
                childBounds.min.y = (y == 0) ? node->bounds.min.y : center.y;
                childBounds.min.z = (z == 0) ? node->bounds.min.z : center.z;

                childBounds.max.x = (x == 0) ? center.x : node->bounds.max.x;
                childBounds.max.y = (y == 0) ? center.y : node->bounds.max.y;
                childBounds.max.z = (z == 0) ? center.z : node->bounds.max.z;

                node->children[i] = octree_createNode(childBounds, node->depth + 1);
            }
        }
    }
}

// Helper function: Checks if container AABB fully contains contained AABB
// (You'll need to define this if not already present)
bool CheckAABBContainsAABB(BoundingBox container, BoundingBox contained) {
    return (contained.min.x >= container.min.x &&
            contained.min.y >= container.min.y &&
            contained.min.z >= container.min.z &&
            contained.max.x <= container.max.x &&
            contained.max.y <= container.max.y &&
            contained.max.z <= container.max.z);
}
Vector3 GetAABBCenter(BoundingBox box) {
    Vector3 center;
    center.x = (box.min.x + box.max.x) / 2.0f;
    center.y = (box.min.y + box.max.y) / 2.0f;
    center.z = (box.min.z + box.max.z) / 2.0f;
    return center;
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
	
	printf("BWEFORE:%i\n",node->objects.count);
    dyanmicarray_resetCount(&node->objects);
	printf("AFTER:%i\n",node->objects.count);
}

void octree_prune(OctreeNode *node) {
    if (node == NULL) {
        return;
    }
	

    // 1. Recursively prune children first (post-order traversal)
    // This ensures child nodes are cleaned up before checking the parent.
    for (int i = 0; i < 8; i++) {
        if (node->children[i] != NULL) {
            octree_prune(node->children[i]);
        }
    }

    // 2. Only consider pruning if this node is an internal node AND is not at max depth.
    // Nodes at max depth are always leaves and won't have children to prune.
    if (node->children[0] != NULL && node->depth < OCTREE_MAX_DEPTH) {
        bool can_prune_children = true;

        // CRITICAL CHECK: Ensure the parent node itself is empty of direct objects.
        // If a node holds straddling objects, it cannot collapse its children.
        if (node->objects.count > 0) {
            TraceLog(LOG_DEBUG, "OCTREE_PRUNE: Node %p (Depth: %d) has %d direct objects. Cannot prune children.",
                     (void*)node, node->depth, node->objects.count);
            can_prune_children = false;
        } else {
            // Now, check if all children are empty leaf nodes.
            for (int i = 0; i < 8; i++) {
                OctreeNode* child = node->children[i];
                if (child == NULL) {
                    // This scenario suggests the child was never created or already freed.
                    // If your subdivision always creates all 8, then this implies an error.
                    // For robust pruning, if any child is NULL or not an empty leaf, we can't prune.
                    can_prune_children = false;

                    break;
                }
                // A child must be a leaf (no children itself) AND contain no objects.
                if (child->children[0] != NULL || child->objects.count > 0) {
                    TraceLog(LOG_DEBUG, "OCTREE_PRUNE: Child %d of node %p (Depth: %d) is not an empty leaf. Cannot prune.",
                             i, (void*)node, node->depth);
                    can_prune_children = false;
                    break;
                }
            }
        }

        if (can_prune_children) {
			printf("HERE\n");
            // All conditions met: parent node is empty, and all children are empty leaves.
            // Proceed to prune (free) the children.
            TraceLog(LOG_DEBUG, "OCTREE_PRUNE: Pruning children of node %p (Depth: %d) - All children are empty leaves.",
                     (void*)node, node->depth);
            for (int i = 0; i < 8; i++) {
                octree_free(&(node->children[i])); // Call your octree_free that also sets to NULL
                node->children[i] = NULL; // Ensure pointer is explicitly NULL after freeing
            }
            // After pruning children, this node effectively becomes a leaf node again.
        }
    }
}

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



void octree_debugDraw(OctreeNode *node) {
    // Base case: If the node is NULL, there's nothing to draw.
    if (node == NULL) {
        return;
    }

    // Draw the bounding box of the current node
    // You can choose different colors based on depth or if it's a leaf node.
    Color boxColor = RED; // Default color

    // Example: Change color for leaf nodes or nodes with objects
    if (node->children[0] == NULL) { // It's a leaf node
        if (node->objects.count > 0) {
            boxColor = GREEN; // Leaf node with objects
        } else {
            boxColor = BLUE; // Empty leaf node
        }
    } else {
        boxColor = YELLOW; // Internal node with children
    }
    
    // Draw the bounding box. Raylib's DrawBoundingBox draws a wireframe.
    DrawBoundingBox(node->bounds, boxColor);

    // Recursively call the function for each child node
    // This will draw the bounding boxes of all descendants.
    for (int i = 0; i < 8; i++) {
        octree_debugDraw(node->children[i]);
    }
}








