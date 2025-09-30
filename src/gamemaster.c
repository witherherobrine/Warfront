



#include "gamemaster.h"


void gamemaster_setupNodes(Entity *entities, int size){
	printf("le caulled");
	int half = size/2;
	EntityNode n1 = (EntityNode){
		.id= 0,
		.parent=0,
		.childCount=0	
	};
	EntityNode n2 = (EntityNode){
		.id= 0,
		.parent=0,
		.childCount=0	
	};
	for(unsigned int i = 0; i < size; i++){
		if(i < half){
			if(i == 0){
				n1.id = i;
			}else{
				entitynode_addChild(&n1, i);
			}
		}else{
			if(i == half){
				n2.id = i;
			}else{
				entitynode_addChild(&n2, i);
			}
		}
	}
	entNodes[0] = n1;
	entNodes[1] = n2;
	
	
    size_t numChildren;
    int* children = entitynode_getChildren(&entNodes[1], &numChildren);
	for (size_t i = 0; i < numChildren; ++i) {
        printf("\nChild ID: %d\n", children[i]);
    }
	// printf("N1: %i", entNodes[1].id);
}
void gamemaster_moveTest(Entity *entities, int nodeId, Vector3 pos){
	
	size_t numChildren;
    int* idArr = entitynode_getChildren(&entNodes[nodeId], &numChildren);
	// int count = sizeof(idArr) / sizeof(int);
	printf("size:%i", numChildren);
	
	
	
	
	// path_manager_moveGroup(entities, idArr, numChildren, pos);
}