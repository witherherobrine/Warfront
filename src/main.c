

/*******************************************************************************************
*
*   raylib [core] example - 3d camera first person
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.3, last time updated with raylib 1.3
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#define FNL_IMPL
#include "raylib.h"
#include "rcamera.h"
#include <stdlib.h>
#include <string.h> // For memset
#include "raymath.h"
#include "FastNoiseLite.h"
fnl_state global_terrain_noise; 




typedef struct{
    Vector3 position;
    Vector3 destination;
    Model model;
    ModelAnimation* anims;
    int currentFrame;
    int animIndex;
}Soldier;

int soldierCount = 64;
Soldier soldiers[64];

float GetHeightAtWorldPosRaw(Image image, Color *pixels, Vector3 pos, Vector3 meshSize) {
    // 1. Map world coordinates to pixel coordinates
    // (pos / meshSize) gives a 0.0 to 1.0 ratio
    int pixelX = (int)((pos.x / meshSize.x) * (image.width - 1));
    int pixelZ = (int)((pos.z / meshSize.z) * (image.height - 1));

    // 2. Clamp values to stay within the image array bounds
    if (pixelX < 0) pixelX = 0;
    if (pixelX >= image.width) pixelX = image.width - 1;
    if (pixelZ < 0) pixelZ = 0;
    if (pixelZ >= image.height) pixelZ = image.height - 1;

    // 3. Get the color and calculate the height
    // .r (Red channel) is standard for heightmaps
    float heightNormalized = (float)pixels[pixelZ * image.width + pixelX].r / 255.0f;

    // 4. Multiply by the vertical scale of your mesh (10 in your case)
    return heightNormalized * meshSize.y;
}


float GetHeightSmooth(Image image, Color *pixels, Vector3 pos, Vector3 meshSize) {
    
    // 1. Map world coordinates to "Pixel Space" as floats
    float pixelX = (pos.x / meshSize.x) * (image.width - 1);
    float pixelZ = (pos.z / meshSize.z) * (image.height - 1);

    // 2. Find the 4 surrounding pixel integer coordinates
    int x0 = (int)pixelX;
    int z0 = (int)pixelZ;
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // 3. Clamp to ensure we don't read outside the array
    
    if(x0 < 0 || z0 < 0 || x1 >= image.width || z1 >= image.height){
        return 0;
    }
    
    if (x1 >= image.width) x1 = image.width - 1;
    if (z1 >= image.height) z1 = image.height - 1;

    // 4. Calculate the fractional part (0.0 to 1.0)
    float fracX = pixelX - x0;
    float fracZ = pixelZ - z0;

    // 5. Get the raw height (0-255) of the 4 neighbors
    float h00 = (float)pixels[z0 * image.width + x0].r;
    float h10 = (float)pixels[z0 * image.width + x1].r;
    float h01 = (float)pixels[z1 * image.width + x0].r;
    float h11 = (float)pixels[z1 * image.width + x1].r;

    // 6. Bilinear Interpolation Formula
    // Blend top two pixels
    float top = h00 + fracX * (h10 - h00);
    // Blend bottom two pixels
    float bottom = h01 + fracX * (h11 - h01);
    // Blend the results vertically
    float finalHeight = top + fracZ * (bottom - top);

    // 7. Normalize (0-1) and scale to mesh height
    return (finalHeight / 255.0f) * meshSize.y;
}

Vector3 WORLD_SIZE = (Vector3){200,100,200};

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Warfront");
    //ToggleFullscreen();
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    MaximizeWindow();
    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 3.5f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;
    
    DisableCursor();                    // Limit cursor to relative movement inside the window
    
    
    Image image = LoadImage("../res/images/groundMap.png");     // Load heightmap image (RAM)
    Color *pixels = LoadImageColors(image);
    Texture2D texture = LoadTextureFromImage(image);        // Convert image to texture (VRAM)

    Mesh mesh = GenMeshHeightmap(image, WORLD_SIZE);
    Model ground = LoadModelFromMesh(mesh);  
    Texture2D groundTexture = LoadTexture("../res/textures/grass.png");
    SetTextureFilter(groundTexture, TEXTURE_FILTER_BILINEAR);
    for(int i = 0; i < ground.materialCount; i++){        
        ground.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;
    }  
    

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    Texture modelTex = LoadTexture("../res/models/person/guyTex.png"); // Set map diffuse texture
    int side = (int)sqrt(soldierCount); // Number of soldiers per row/column

    Shader skinningShader = LoadShader("../res/shaders/skinning.vs","../res/shaders/skinning.fs");

    for(int i = 0; i < soldierCount; i++){
        Model model = LoadModel("../res/models/person/disgiuy.m3d");
        for(int i = 0; i < model.materialCount; i++){
            model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = modelTex;
            model.materials[i].shader = skinningShader;
        }   
        int animsCount = 3;
        ModelAnimation* anim = LoadModelAnimations("../res/models/person/disgiuy.m3d", &animsCount);
        
        soldiers[i].model = model;
        soldiers[i].anims = anim;
        
        float x = GetRandomValue(0,WORLD_SIZE.x);
        float z = GetRandomValue(0,WORLD_SIZE.z);

        soldiers[i].position = (Vector3){x, GetHeightAtWorldPosRaw(image, pixels, (Vector3){ x, 0.0f, z }, WORLD_SIZE)+2, z};
        soldiers[i].currentFrame = GetRandomValue(0,50);
        soldiers[i].animIndex = GetRandomValue(0,2);
    }
    
    Shader instanceShader = LoadShader("../res/shaders/instancing.vs","../res/shaders/instancing.fs");
    Model tree = LoadModel("../res/models/tree/scene.gltf");
    for(int i = 0; i < tree.materialCount; i++){
        tree.materials[i].shader = instanceShader;
    }   


    int MAX_TREES = 300;
    Matrix treeTransforms[MAX_TREES];
    
    for (int i = 0; i < MAX_TREES; i++) {
        float ranGirth = GetRandomValue(1.0f,30.0f);
        float ranHeight = GetRandomValue(20.0f,40.0f);
        float x = GetRandomValue(0,WORLD_SIZE.x);
        float z = GetRandomValue(0,WORLD_SIZE.z);
        float groundHeight = GetHeightAtWorldPosRaw(image, pixels, (Vector3){ x, 0.0f, z }, WORLD_SIZE);
        treeTransforms[i] = MatrixTranslate(x,groundHeight-10,z);
        float randomRotation = GetRandomValue(0, 360) * DEG2RAD;
        Matrix rotation = MatrixRotateY(randomRotation);
        Matrix scale = MatrixScale(ranGirth,ranHeight,ranGirth);
        treeTransforms[i]  = MatrixMultiply(MatrixMultiply(rotation, scale),treeTransforms[i]);
    }

    unsigned int animIndex = 0;         // Current animation playing
    float animCurrentFrame = 0.0f;      // Current animation frame (supporting interpolated frames)
    
    
    
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        UpdateCamera(&camera, cameraMode);                  // Update camera
        
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            
            float newDist = GetHeightSmooth(image, pixels, camera.position,WORLD_SIZE)+3.5f;
            CameraMoveUp(&camera,newDist - camera.position.y);
            for(int i = 0; i < soldierCount; i++){
                
                soldiers[i].currentFrame += 1.0f;
                if (soldiers[i].currentFrame >= soldiers[i].anims[soldiers[i].animIndex].keyframeCount) soldiers[i].currentFrame = 0.0f;
                UpdateModelAnimation(soldiers[i].model, soldiers[i].anims[soldiers[i].animIndex], soldiers[i].currentFrame);
                
                DrawModel(soldiers[i].model,soldiers[i].position ,1.0f,WHITE);
            }
                
            DrawModel(ground, (Vector3){0,0,0},1.0f,WHITE);
            DrawMeshInstanced(tree.meshes[0], tree.materials[1], treeTransforms, MAX_TREES);
            DrawMeshInstanced(tree.meshes[1], tree.materials[2], treeTransforms, MAX_TREES);
            
            //for (int i = 0; i < MAX_TREES; i++) {
                //DrawMesh(tree.meshes[0], tree.materials[1], treeTransforms[i]);
            //}
            EndMode3D();

            // Draw info boxes
            DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 5, 330, 100, BLUE);



            DrawRectangle(600, 5, 195, 100, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(600, 5, 195, 100, BLUE);

            DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z), 610, 60, 10, BLACK);
            DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z), 610, 75, 10, BLACK);
            DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z), 610, 90, 10, BLACK);
            
            DrawFPS(30,30);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
