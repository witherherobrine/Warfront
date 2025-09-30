

#include "terrain.h"
#include <stdio.h>
#include <stdlib.h>

#define GRAY_VALUE(c) ((float)(c.r + c.g + c.b)/3.0f)

extern Color* pixels;

extern int MAP_WIDTH;
extern int MAP_HEIGHT;


Mesh createTerrain(const char* imagePath, float mapSize, float mapHeight) {
    Image groundMap = LoadImage(imagePath);
    Mesh ground = GenMeshHeightmap(groundMap, (Vector3){mapSize, mapHeight, mapSize});
    UnloadImage(groundMap); // Unload the image after creating the mesh

    float* newTexCoords = (float*)RL_MALLOC(ground.vertexCount * 2 * sizeof(float));
    for (int i = 0; i < ground.vertexCount; i++) {
//        newTexCoords[i * 2] = ground.vertices[i * 3];
//        newTexCoords[i * 2 + 1] = ground.vertices[i * 3 + 2];
        newTexCoords[i * 2] = ground.vertices[i * 3] / 40.0f;
        newTexCoords[i * 2 + 1] = ground.vertices[i * 3 + 2] / 40.0f;
		if(i < 100){
//			printf("data: %f\n",ground.vertices[i]);
		}
    }
    UpdateMeshBuffer(ground, 1, newTexCoords, ground.vertexCount * 2 * sizeof(float), 0);
    RL_FREE(newTexCoords); //Free the allocated texcoords

    return ground;
}

static float GetHeightFromPixel(Color pixel) {
    return (float)pixel.r;
}
static Vector2 WorldToPixelCoords(Vector3 worldPos) {
    float x = worldPos.x / MAP_WIDTH;
    float z = worldPos.z / MAP_WIDTH;

    if (x < 0 || x >= MAP_WIDTH || z < 0 || z >= MAP_WIDTH) {
        return (Vector2){-1, -1}; // Return an invalid coordinate if out of bounds
    }

    return (Vector2){x, z};
}

bool terrain_terrainPointCollide(Color* pixels, Bullet* bullet) {
	
	// DrawLine3D(bulletData->ray.position, Vector3Add(bulletData->ray.position, Vector3Scale(bulletData->ray.direction, bulletData->velocity)), YELLOW);

    Vector3 p1 = bullet->ray.position;
    Vector3 p2 = Vector3Add(bullet->ray.position, Vector3Scale(bullet->ray.direction, bullet->velocity));
    
    Vector2 p1_pixel = WorldToPixelCoords(p1);
    Vector2 p2_pixel = WorldToPixelCoords(p2);
    
    // If either point is out of the map's horizontal bounds, no collision.
    // We can be more aggressive here. If either point is invalid, the bullet is entirely out.
    if (p1_pixel.x == -1.0f || p2_pixel.x == -1.0f) {
        return false;
    }

    // 1. Determine the pixel bounding box
    int min_x = (int)fminf(p1_pixel.x, p2_pixel.x);
    int max_x = (int)fmaxf(p1_pixel.x, p2_pixel.x);
    int min_z = (int)fminf(p1_pixel.y, p2_pixel.y); // y in pixel coords is z in world coords
    int max_z = (int)fmaxf(p1_pixel.y, p2_pixel.y);

    // 2. Clamp the bounding box to the terrain dimensions
    min_x = fmaxf(0, min_x);
    max_x = fminf(MAP_WIDTH - 1, max_x);
    min_z = fmaxf(0, min_z);
    max_z = fminf(MAP_WIDTH - 1, max_z);

    // 3. Iterate over every pixel in the bounding box
    for (int pixel_z = min_z; pixel_z <= max_z; ++pixel_z) {
        for (int pixel_x = min_x; pixel_x <= max_x; ++pixel_x) {
            // Get the world coordinates for the current pixel
            float world_x = (float)pixel_x * 1;
            float world_z = (float)pixel_z * 1;

            // 4. Find the bullet's y-position at this (x, z)
            float t;
            float dx = p2.x - p1.x;
            float dz = p2.z - p1.z;
            
            // Avoid division by zero
            if (fabsf(dx) > fabsf(dz)) {
                // Use x to find t
                if (fabsf(dx) < EPSILON) continue; // Bullet is perfectly vertical, can't get t from x
                t = (world_x - p1.x) / dx;
            } else {
                // Use z to find t
                if (fabsf(dz) < EPSILON) continue; // Bullet is perfectly horizontal, can't get t from z
                t = (world_z - p1.z) / dz;
            }
            
            float bulletY = p1.y + (p2.y - p1.y) * t;

            // 5. Check for a collision
            Color pixel = pixels[pixel_z * MAP_WIDTH + pixel_x];
            float terrainHeight = GetHeightFromPixel(pixel);
            
//            if (terrainHeight * 1 > p2.y) {
//                printf("COLLIDE! B-%f T-%f",p2.y, terrainHeight);
//                // Collision! We can return immediately.
//                return true;
//            }
        if(yPointXZTest((Vector3){200,50,200},p2.x,p2.z) > p2.y){
            return true;
        }
        }
    }
    
    // No collision found after checking all relevant pixels.
    return false;
}
float yPointXZTest(Vector3 size, float x, float z)
{
    // int mapX = heightmap.width;
    // int mapZ = heightmap.height;
	
	int mapX = MAP_WIDTH;
	int mapZ = MAP_HEIGHT;
	
    // Color *pixels = LoadImageColors(heightmap); // Need to load pixels here

    // if (pixels == NULL || mapX <= 1 || mapZ <= 1)
    // {
        // UnloadImageColors(pixels);
        // return 0.0f; // Or some default height, handle error appropriately
    // }

    Vector3 scaleFactor = { size.x / (mapX - 1), size.y / 255.0f, size.z / (mapZ - 1) };

    // Normalize x and z to the range [0, 1] based on the terrain size
    float normalizedX = x / size.x;
    float normalizedZ = z / size.z;

    // Scale normalized coordinates to the heightmap dimensions
    float pixelXFloat = normalizedX * (mapX - 1);
    float pixelZFloat = normalizedZ * (mapZ - 1);

    // Get integer pixel coordinates
    int pixelX = (int)floorf(pixelXFloat);
    int pixelZ = (int)floorf(pixelZFloat);

    // Handle cases outside the heightmap bounds
    if (pixelX < 0 || pixelX >= mapX - 1 || pixelZ < 0 || pixelZ >= mapZ - 1)
    {
        // UnloadImageColors(pixels);
        return 0.0f; // Or some default height, handle boundary conditions
    }

    // Get the gray values of the four neighboring pixels
    float y00 = GRAY_VALUE(pixels[pixelX + pixelZ * mapX]) * scaleFactor.y;
    float y01 = GRAY_VALUE(pixels[pixelX + (pixelZ + 1) * mapX]) * scaleFactor.y;
    float y10 = GRAY_VALUE(pixels[(pixelX + 1) + pixelZ * mapX]) * scaleFactor.y;
    float y11 = GRAY_VALUE(pixels[(pixelX + 1) + (pixelZ + 1) * mapX]) * scaleFactor.y;

    // Calculate interpolation factors
    float fracX = pixelXFloat - pixelX;
    float fracZ = pixelZFloat - pixelZ;

    // Bilinear interpolation
    float y = (1 - fracX) * (1 - fracZ) * y00 +
              fracX * (1 - fracZ) * y10 +
              (1 - fracX) * fracZ * y01 +
              fracX * fracZ * y11;

    // UnloadImageColors(pixels);
    return y;
}

