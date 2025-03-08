

#include <stdio.h>
#include "raylib.h"
#include "raymath.h"




typedef struct Bullet{
	int id;
	Ray ray;
	float velocity;
	bool enabled;
	Vector3 origin;
} Bullet;

Bullet bullet_initBullet();
void bullet_initBulletCache();
void bullet_updateBullets();
void bullet_updateBullet(Bullet* b);
void bullet_fireBullet(Vector3 start, Vector3 direction, float velocity, int shooterId);