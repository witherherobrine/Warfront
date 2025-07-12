

#include "bullet.h"


const int bulletCacheAmount = 1000;
int bulletFireIndex = 0;
Bullet* bullets[1000];

int MAX_BULLET_DIST = 1000;

void bullet_updateBullet(Bullet* b){
	if(!b->enabled){return;}
	if(Vector3Distance(b->origin, b->ray.position) > MAX_BULLET_DIST){
		b->enabled = false;
		b->velocity = 0;
	}
	
	b->ray.position = Vector3Add(b->ray.position,Vector3Add(Vector3Scale(b->ray.direction,b->velocity),(Vector3){0,-.001,0}));

}

void bullet_fireBullet(Vector3 start, Vector3 direction, float velocity, int shooterId){
	
	bullets[bulletFireIndex]->id = shooterId;
	
	bullets[bulletFireIndex]->ray.position = start;
	bullets[bulletFireIndex]->ray.direction = direction;
	bullets[bulletFireIndex]->velocity = velocity;
	bullets[bulletFireIndex]->enabled = true;
	bullets[bulletFireIndex]->origin = start;
	// bullets[bulletFireIndex]->enabled = false;
	
	bulletFireIndex++;
	if(bulletFireIndex >= bulletCacheAmount){
		bulletFireIndex = 0;
	}
}