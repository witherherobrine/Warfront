
#ifndef BLOCK_H
#define BLOCK_H


#include "raylib.h"

Block blocks[3];

// BoundingBox
typedef struct Block{
	Vector3 position;
	float length;
	float width;
	float height;
} Block;

#endif