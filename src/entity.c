


#include "raylib.h"
#include "entity.h"
#include "terrain.h"
#include <stdio.h>
#include <stdlib.h>

float length = 0.5f;
float width = 0.5f;
float height = 2.2f;


int noLoopAnims[] = {0};

const float aabbSize = 20.0f;
BoundingBox testaabb = (BoundingBox){
	.min={-aabbSize,-aabbSize,-aabbSize},
	.max={aabbSize,aabbSize,aabbSize}
};

// Entity entity_createEntity(){
	
    // int animsCount = 2;
	// Model model = LoadModel("../res/models/person/disgiuy.m3d");
	
	// printf("ENTITY SHADER ID - %i\n", model.materials[0].shader.id);
	// printf("ENTITY MATERIALS - %i\n", model.materialCount);
	// printf("ENTITY MESHES - %i\n", model.meshCount);
	
	
		// entities[i].model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("../res/textures/grass.png"); // Set map diffuse texture
	
    // bool validModel =  IsModelValid(model); 
	// if(!validModel){	
		// printf("CRITICAL: INVALID MODEL");
		// exit(1);	
	// }
	
	// if(!model.meshCount > 0 || model.meshes == NULL){
		
		// printf("CRITICAL: INVALID MODEL - 0 MESHES");
		// exit(1);	
	// }
	
	// ModelAnimation* anim = LoadModelAnimations("../res/models/person/disgiuy.m3d", &animsCount);
	// bool validAnim;
	// for(unsigned int i = 0; i < animsCount; i++){	
		// validAnim =IsModelAnimationValid(model, anim[0]);
	// }
	
	// if(!validAnim){	
		// printf("CRITICAL: INVALID ANIMATION");
		// exit(1);	
	// }
	

	// Entity e = (Entity){
		// .id = 0,
		// .player = false,
		// .position = (Vector3){(float)GetRandomValue(-15, 15)/0.5f,2.3f,(float)GetRandomValue(-15, 15)/0.5f},
		// .rotation = 0.0f,
		// .velocity = (Vector3){0,0,0},
		// .destination = (Vector3){0,0,0},
		// .aabb = (BoundingBox){1},
		// .model = model,
		// .alive = true,
		// .animFrame = 0,
		// .animIndex = 0,
		// .anims = anim
	// };
	// entity_moveEntity(&e,e.position);
	
	// return e;
// }

void entity_updateEntityShader(Entity* e, Shader shader){
	Model model = e->model;
	for (int i = 0; i < model.materialCount; i++) {
		model.materials[i].shader = shader;
	}
}

void entity_DrawEntity(Entity e){
		DrawModel(e.model, e.position,1.0f, WHITE);
}
void entity_updateEntity(Entity* e){
	
	
	if(!e->alive && e->animFrame >= e->anims[e->animIndex].frameCount-1){
		return;
	}
	    e->model.transform = MatrixMultiply(MatrixRotateY(e->rotation),MatrixTranslate(e->position.x, e->position.y, e->position.z));
	float newDist = yPointXZTest((Vector3){200,50,200},e->position.x, e->position.z)+2;
	e->position.y = newDist;

	// if(!e->player && e->alive){
		// if(Vector2Length((Vector2){e->velocity.x,e->velocity.z}) > 0 ){
			// entity_switchAnimation(e,2);
		// }else{
			// entity_switchAnimation(e,1);
		// }
	// }
	
	e->velocity = (Vector3){0,0,0};
	if(e->alive && !atPosition(e->position, e->destination)){
		
		float dx = e->destination.x - e->position.x;
		float dy = e->destination.z - e->position.z;
		float length = sqrt((dx*dx)+(dy*dy));
		dx /= length;
		dy /= length;
		dx *=.1f;
		dy *=.1f;
		e->velocity = (Vector3){dx,0,dy};
		e->position = Vector3Add(e->position, e->velocity);
	}
	
	if(e->alive && atPosition(e->position, e->destination)){
		entity_moveEntity(e, (Vector3){GetRandomValue(1,200),0,GetRandomValue(1,200)});
	}
	
	// e->aabb.min = (Vector3){e->position.x-length,e->position.y-height,e->position.z-width};
	// e->aabb.max = (Vector3){e->position.x+length,e->position.y+height,e->position.z+width};
	
	UpdateModelAnimation(e->model, e->anims[e->animIndex], e->animFrame);
	
	
	e->animFrame++;
	// if(!e->alive && e->animFrame >= e->anims[e->animIndex].frameCount-2){
		 // e->animFrame = e->animFrame = e->anims[e->animIndex].frameCount-2;
	// }
	if(e->animFrame >= e->anims[e->animIndex].frameCount){
		e-> animFrame = 0;
	}
	
	// if(!e->alive){
		 // e->animIndex = 0;
	// }
}

void entity_moveEntity(Entity* e, Vector3 pos){
	e->destination = pos;    
	
	Vector3 direction = {pos.x - e->position.x, 0.0f, pos.z - e->position.z};
	e->rotation = atan2f(direction.x, direction.z)- M_PI/2;
	// e->rotation = (180/M_PI)*atan2f(direction.x, direction.z) - 90.0f;
}

void entity_queryBox(Entity* e, Vector3 position){
	
	const float half = aabbSize/2.0f;
	testaabb.min = Vector3AddValue(position, -half);
	testaabb.max = Vector3AddValue(position, half);
	
}
void entity_setEntityTarget(Entity* e, Vector3 target){
	
	Vector3 direction = {target.x - e->position.x, 0.0f, target.z - e->position.z}; 
	e->rotation = atan2f(direction.x, direction.z)- M_PI/2;
}

void entity_switchAnimation(Entity* e, int index){
	e->animIndex = index;
	// e->animFrame = 0;
}
static bool atPosition(Vector3 ePos, Vector3 v){
	const float buffer = 0.1f;
	if(ePos.x > v.x - buffer && ePos.x < v.x + buffer && ePos.z > v.z - buffer && ePos.z < v.z + buffer){
		return true;
	}
	return false;
}

