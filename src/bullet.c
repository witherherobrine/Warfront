

#include "bullet.h"


const int bulletCacheAmount = 1000;
int bulletFireIndex = 0;
Bullet bullets[1000];

Bullet bullet_initBullet(){
		
	Bullet b = {
		id:-1,
		ray: (Ray){
			position: (Vector3){0,0,0},
			direction: (Vector3){0,1,0}
		},
		velocity:4.0f,
		enabled:false,
		origin:(Vector3){0,0,0}
		
	};
	return b;
}

void bullet_initBulletCache(){
	for(unsigned int i = 0; i < bulletCacheAmount; i++){
		bullets[i] = bullet_initBullet();
	}
}

void bullet_updateBullet(Bullet* b){
	if(!b->enabled){return;}
	if(Vector3Distance(b->origin, b->ray.position) > 100.0f){
		b->enabled = false;
		b->velocity = 0;
	}
}

void bullet_fireBullet(Vector3 start, Vector3 direction, float velocity, int shooterId){
	
	bullets[bulletFireIndex].id = shooterId;
	
	bullets[bulletFireIndex].ray.position = start;
	bullets[bulletFireIndex].ray.direction = direction;
	bullets[bulletFireIndex].velocity = velocity;
	bullets[bulletFireIndex].enabled = true;
	bullets[bulletFireIndex].origin = start;
	
	bulletFireIndex++;
	if(bulletFireIndex >= bulletCacheAmount){
		bulletFireIndex = 0;
	}
}