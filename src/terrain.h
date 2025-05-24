

#ifndef TERRAIN_H
#define TERRAIN_H

#include "raylib.h"
#include "raymath.h"
#include <math.h>

typedef struct {
    float y;
    Vector3 normal;
} TerrainInfo;


Mesh createTerrain(const char* imagePath, float mapSize, float mapHeight);
// float getYPointAtXZ(Mesh terrain, float x, float z);
float getYPointAtXZ(Mesh terrain, float x, float z, int imgWidth, int imgHeight, int tWidth, int tHeight);
float yPointXZTest(Vector3 size, float x, float z);

#endif