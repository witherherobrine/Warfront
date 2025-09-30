


#include "entitynode.h"



EntityNode* entitynode_createEntityNode(int id, int parent) {
    EntityNode* node = (EntityNode*)malloc(sizeof(EntityNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1); 
    }

    node->id = id;
	if(parent != -1){
		node->parent = parent;
	}
    node->children = NULL; 
    node->childCount = 0;
    return node;
}


// Function to add a child to an EntityNode
void entitynode_addChild(EntityNode* node, int childId) {
    node->childCount++;
    node->children = (int*)realloc(node->children, node->childCount * sizeof(int));
    if (node->children == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1); 
    }
    node->children[node->childCount - 1] = childId;
}
int* entitynode_getChildren(const EntityNode* node, size_t* numChildren) {
    *numChildren = node->childCount; 
    return node->children;
}

// Function to free the memory allocated for an EntityNode
void entitynode_freeEntityNode(EntityNode* node) {
    if (node->children != NULL) {
        free(node->children); 
    }
    free(node);
}

