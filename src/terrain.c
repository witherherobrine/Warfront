

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
        newTexCoords[i * 2] = ground.vertices[i * 3];
        newTexCoords[i * 2 + 1] = ground.vertices[i * 3 + 2];
		if(i < 100){
			printf("data: %f\n",ground.vertices[i]);
		}
    }
    UpdateMeshBuffer(ground, 1, newTexCoords, ground.vertexCount * 2 * sizeof(float), 0);
    RL_FREE(newTexCoords); //Free the allocated texcoords

    return ground;
}




// float getYPointAtXZ(Mesh terrain, float x, float z, int imgWidth, int imgHeight, int tWidth, int tHeight) {
	
    // float normalizedX = x / tWidth;
    // float normalizedZ = z / tWidth;
	// printf("nX:%f",normalizedX);

    // float gridX = normalizedX * (imgWidth - 1);
    // float gridZ = normalizedZ * (imgWidth - 1);

    // int x0 = (int)gridX;
    // int z0 = (int)gridZ;
	
	// printf("X:%i, Z:%i\n",x0,z0);

    // x0 = (x0 >= imgWidth) ? imgWidth - 1 : x0;
    // z0 = (z0 >= imgWidth) ? imgWidth - 1 : z0;

    // int index = x0 * imgWidth;
	
	// printf("INDEX-%i\n",index);

    // float y = terrain.vertices[index * 3 + 1];
	// printf("y ting -%f\n",y);

    // return y;

// } 
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

