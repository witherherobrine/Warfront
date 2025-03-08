


#include "pathmanager.h"


void path_manager_moveIndividual(Entity* e){}

void path_manager_moveGroup(Entity *entities, int *idArr, size_t entCount, Vector3 position){
	printf("walkupdate");
	for(size_t i = 0; i < entCount; i++){
		// entity_moveEntity(&entities[i], (Vector3){GetRandomValue(-64,64),0,GetRandomValue(-64,64)});
		// entitymanager_setEntityTarget(&entities[idArr[i]],position);
		position.x = position.x + i;
		entity_moveEntity(&entities[idArr[i]], position);
	}
}