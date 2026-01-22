

#define FNL_IMPL
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"       // For rlLoadVertexBuffer, rlDrawVertexArray, etc.

// Use Raylib's internal GLAD/OpenGL definitions
#define GLSL_VERSION            330


#include "utils.h"


#include "game.h"
#include "entities.h"
#include "systems.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


Shader terrainShader;
int lightDirLoc, lightColorLoc, viewPosLoc, ambientLoc;



void SetupShaders() {

    terrainShader = LoadShader("../res/shaders/lighting.vs", "../res/shaders/lighting.fs");

    lightDirLoc = GetShaderLocation(terrainShader, "lightDir");
    lightColorLoc = GetShaderLocation(terrainShader, "lightColor");
    viewPosLoc = GetShaderLocation(terrainShader, "viewPos");
    ambientLoc = GetShaderLocation(terrainShader, "ambientStrength");
    
    float ambientStrength = 0.2f; 
    SetShaderValue(terrainShader, ambientLoc, &ambientStrength, SHADER_UNIFORM_FLOAT);
}


void SetupEntities() {
    
    Texture modelTex = LoadTexture("../res/models/person/guyTex.png"); // Set map diffuse texture


    Model model = LoadModel("../res/models/person/disgiuy.m3d");
    for(int i = 0; i < model.materialCount; i++){
            
        model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = modelTex;
        model.materials[i].shader = terrainShader;        
    }
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = modelTex;
    //model.materials[0].shader = terrainShader;        
    int animsCount = 2;
    ModelAnimation* anim = LoadModelAnimations("../res/models/person/disgiuy.m3d", &animsCount);


    
    EntityId cube2 = create_entity();
    attach_transform(cube2, 
        (Vector3){0,0,0}, // position
        (Vector3){0.0f, 1.0f, 0.0f}, // rotationAxis (Y-axis)
        0.0f,                         // rotationAngle (0 degrees)
        (Vector3){1,1,1}); // scale
    attach_render(cube2, model);
    attach_animations(cube2, anim);
    //attach_soldier(cube2);
    attach_player_tag(cube2);
    
    
    
    EntityId playerCam = create_entity();
       attach_transform(playerCam, 
        (Vector3){0.0f, 1.8f, 0.0f}, // Initial position (head height)
        (Vector3){0.0f, 1.0f, 0.0f}, // Rotation axis
        0.0f, 
        (Vector3){1.0f, 1.0f, 1.0f});
    attach_camera(playerCam, 70.0f, 0.0f, 0.0f, true);
    SetActiveCamera(playerCam); // <-- Call the setter to initialize the active ID
    attach_parent(playerCam, cube2, (Vector3){0.0f, 1.5f, 0.0f});
    
    
    for(int i = 0; i < 30; i++){
        //Model model2 = LoadModel("../res/models/person/disgiuy.m3d");
        //model2.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = modelTex;
                
        EntityId e = create_entity();
        
        attach_transform(e, 
            (Vector3){(0.0f), 0.0f, 0.0f}, // position
            (Vector3){0.0f, 1.0f, 0.0f}, // rotationAxis (Y-axis)
            0.0f,                         // rotationAngle (0 degrees)
            (Vector3){1.0f, 1.0f, 1.0f}); // scale
        attach_render(e, model);
        attach_animations(e, anim);
        attach_soldier(e);
    }
}

Matrix* BakeAnimationFrames(Model model, ModelAnimation anim, int* outMatrixCount) {
    int frameCount = anim.frameCount;
    int boneCount = anim.boneCount;
    *outMatrixCount = frameCount * boneCount;

    Matrix* bakedMatrices = (Matrix*)RL_MALLOC(frameCount * boneCount * sizeof(Matrix));
    Matrix* worldTransforms = (Matrix*)RL_MALLOC(boneCount * sizeof(Matrix));
    Matrix* globalBindPose = (Matrix*)RL_MALLOC(boneCount * sizeof(Matrix));

    // --- STEP 1: Calculate Global Bind Pose Matrices ---
    // We must do this because invBindPose needs to be the inverse of the GLOBAL bind pose
    for (int i = 0; i < boneCount; i++) {
        Matrix localBind = QuaternionToMatrix(model.bindPose[i].rotation);
        localBind.m12 = model.bindPose[i].translation.x;
        localBind.m13 = model.bindPose[i].translation.y;
        localBind.m14 = model.bindPose[i].translation.z;

        int parentIndex = model.bones[i].parent;
        if (parentIndex == -1) globalBindPose[i] = localBind;
        //else globalBindPose[i] = MatrixMultiply(localBind, globalBindPose[parentIndex]);
        else globalBindPose[i] = MatrixMultiply(globalBindPose[parentIndex], localBind);
        
        
    }

    // --- STEP 2: Bake the Animation Frames ---
    for (int i = 0; i < frameCount; i++) {
        for (int j = 0; j < boneCount; j++) {
            
            
     
            
            // Local animated transform
            Matrix localMat = QuaternionToMatrix(anim.framePoses[i][j].rotation);
            localMat.m12 = anim.framePoses[i][j].translation.x;
            localMat.m13 = anim.framePoses[i][j].translation.y;
            localMat.m14 = anim.framePoses[i][j].translation.z;

            // Global animated transform
            int parentIndex = anim.bones[j].parent;
            if (parentIndex == -1) worldTransforms[j] = localMat;
            //else worldTransforms[j] = MatrixMultiply(localMat, worldTransforms[parentIndex]);
            else worldTransforms[j] = MatrixMultiply(worldTransforms[parentIndex], localMat);
            
            

            // Final Matrix = Inverse(GlobalBindPose) * GlobalAnimatedPose
            Matrix invBindPose = MatrixInvert(globalBindPose[j]);
            
            // RAYLIB NOTE: MatrixMultiply order matters! 
            // Usually: BoneSpaceTransform = InvBind * WorldAnim
            bakedMatrices[i * boneCount + j] = MatrixMultiply(invBindPose, worldTransforms[j]);
            //bakedMatrices[i * boneCount + j] = MatrixMultiply(worldTransforms[j], invBindPose);
                               // Inside BakeAnimationFrames loop, pick a bone index (e.g., 5)
            if (j == 5 && i == 0) { // Bone 5, Frame 0
                printf("Bone 5 Global Bind Matrix (m12, m13, m14): %.2f, %.2f, %.2f\n", 
                       globalBindPose[j].m12, globalBindPose[j].m13, globalBindPose[j].m14);
                printf("Bone 5 Baked Matrix Row 0: %.2f, %.2f, %.2f, %.2f\n", 
                       bakedMatrices[j].m0, bakedMatrices[j].m4, bakedMatrices[j].m8, bakedMatrices[j].m12);
                       
            }
        }
    }

    RL_FREE(worldTransforms);
    RL_FREE(globalBindPose);
    return bakedMatrices;
}

Texture2D CreateAnimationTexture(Matrix *bakedMatrices, int boneCount, int frameCount) {
    // Each matrix is 4 pixels wide (each pixel is one row of the matrix)
    int width = boneCount * 4; 
    int height = frameCount;

    Image animImage = {
        .data = bakedMatrices,      // Your Matrix* array
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
    };
    
    //bool ex = ExportImage(animImage, "imgexport.png");
    
    //printf("EXPORT WORKIE %i\n",ex);
    
    Texture2D animTexture = LoadTextureFromImage(animImage);
    
    // CRITICAL: We need Nearest filtering. 
    // Bilinear filtering will "blend" bones together and ruin the math.
    SetTextureFilter(animTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(animTexture, TEXTURE_WRAP_CLAMP);

    return animTexture;
}


int tframe = 0;

int main(void) {
    time_t time1, time2;
    time(&time1);
    init_game_world();
    
    const int screenWidth = 1920;
    const int screenHeight = 1000;
    
    InitWindow(screenWidth, screenHeight, "ECS Raylib Soldier Example");
    double sTime = GetTime();
    SetTargetFPS(60);
    SetupShaders();
    SetupEntities();
    TerrainSetupSystem(terrainShader);
    DisableCursor();


    
    //-----
    
    Model model = LoadModel("../res/models/person/disgiuy.m3d");
    
    for (int i = 0; i < 5; i++) { // Check first 5 vertices
    int idx = i * 4;
    float sum = model.meshes[0].boneWeights[idx] + 
                model.meshes[0].boneWeights[idx+1] + 
                model.meshes[0].boneWeights[idx+2] + 
                model.meshes[0].boneWeights[idx+3];
                
    printf("Vertex %d: IDs(%d, %d, %d, %d) Weights(%.2f, %.2f, %.2f, %.2f) Sum: %.2f\n", 
        i, 
        model.meshes[0].boneIds[idx], model.meshes[0].boneIds[idx+1],
        model.meshes[0].boneIds[idx+2], model.meshes[0].boneIds[idx+3],
        model.meshes[0].boneWeights[idx], model.meshes[0].boneWeights[idx+1],
        model.meshes[0].boneWeights[idx+2], model.meshes[0].boneWeights[idx+3],
        sum);
}
    
    Texture modelTex = LoadTexture("../res/models/person/guyTex.png"); // Set map diffuse texture
    for(int i = 0; i < model.materialCount; i++){        
        model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = modelTex;
    }

    if (model.meshes[0].boneIds == NULL) {
    printf("ERROR: model.meshes[0].boneIds is NULL. The file didn't load bone data!\n");
}
    int maxId = 0;
    for (int i = 0; i < model.meshes[0].vertexCount * 4; i++) {
        if(model.meshes[0].boneIds[i] == 255) continue;
        if (model.meshes[0].boneIds[i] > maxId) maxId = (int)model.meshes[0].boneIds[i];
    }
    printf("MAX BONE ID FOUND IN MESH: %i\n", maxId);
    
    
    int animsCount = 2;
    ModelAnimation* anim = LoadModelAnimations("../res/models/person/disgiuy.m3d", &animsCount);

    int outM;
    Matrix* bakedMatrices = BakeAnimationFrames(model, anim[0], &outM);
    Texture2D bakedTex = CreateAnimationTexture(bakedMatrices, anim->boneCount, anim->frameCount);
    printf("Texture Width: %i, Expected: %i\n", bakedTex.width, anim->boneCount * 4);
    // After CreateAnimationTexture
printf("Texture Info: Width %d (Expected %d), Height %d (Expected %d)\n", 
       bakedTex.width, anim[0].boneCount * 4, 
       bakedTex.height, anim[0].frameCount);

    Shader animShader = LoadShader("../res/shaders/modelanim.vs", "../res/shaders/modelanim.fs");
    animShader.locs[SHADER_LOC_VERTEX_POSITION] = GetShaderLocationAttrib(animShader, "vertexPos");
animShader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = GetShaderLocationAttrib(animShader, "vertexTexCoord");
    model.materials[0].shader = animShader;

    // 1. Get locations
    int boneIdsLoc = GetShaderLocationAttrib(animShader, "vertexBoneIds");
    int boneWeightsLoc = GetShaderLocationAttrib(animShader, "vertexBoneWeights");

    printf("Shader Location IDs: %i, %i\n", boneIdsLoc, boneWeightsLoc);
    
    // In main() after SetupShaders
printf("Location - Position: %d\n", GetShaderLocationAttrib(animShader, "vertexPos"));
printf("Location - BoneIds: %d (VBO ID: %u)\n", boneIdsLoc, model.meshes[0].vboId[boneIdsLoc]);
printf("Location - BoneWeights: %d (VBO ID: %u)\n", boneWeightsLoc, model.meshes[0].vboId[boneWeightsLoc]);

// Bone IDs
if (boneIdsLoc != -1 && model.meshes[0].boneIds != NULL) {
    model.meshes[0].vboId[boneIdsLoc] = rlLoadVertexBuffer(
        model.meshes[0].boneIds, 
        model.meshes[0].vertexCount * 4 * sizeof(float), 
        false
    );
    rlEnableVertexAttribute(boneIdsLoc);
    // Use 0x1401 (GL_UNSIGNED_BYTE). 
    // IMPORTANT: Use rlSetVertexAttributeDefault or ensure index is correct
    rlSetVertexAttribute(boneIdsLoc, 4, 0x1401, false, 0, 0); 
}

// Bone Weights
if (boneWeightsLoc != -1 && model.meshes[0].boneWeights != NULL) {
    model.meshes[0].vboId[boneWeightsLoc] = rlLoadVertexBuffer(
        model.meshes[0].boneWeights, 
        model.meshes[0].vertexCount * 4 * sizeof(float), 
        false
    );
    rlEnableVertexAttribute(boneWeightsLoc);
    rlSetVertexAttribute(boneWeightsLoc, 4, 0x1406, false, 0, 0); // 0x1406 is GL_FLOAT
}
    

    // 3. DONE. Do NOT touch the vboId array after this point.
    printf("Final Bone VBO: %i\n", model.meshes[0].vboId[boneIdsLoc]);
    
    int MAX_INSTANCES = 100;
    // 6. Setup Instances
    Matrix renderTransforms[MAX_INSTANCES];
    float instanceFrames[MAX_INSTANCES];
    for (int i = 0; i < MAX_INSTANCES; i++) {
        instanceFrames[i] = (float)GetRandomValue(0, anim[0].frameCount - 1);
        //instanceFrames[i] = 0;
    }
    
    //-----


    //rlDisableBackfaceCulling();



    Texture grass = LoadTexture("../res/textures/grass.png");

    while (!WindowShouldClose()) {
        
        float deltaTime = GetFrameTime();
        
        PlayerInputSystem(deltaTime);
        temporaryAIMoveSystem();
        ParentingSystem();
        CameraSystem(deltaTime); 
        ViewDistanceManagerSystem(grass,terrainShader);
        tframe++;
        if(tframe > 40){
            tframe = 0;
            }
            
            for (int i = 0; i < MAX_INSTANCES; i++) {
                instanceFrames[i] += GetFrameTime() * 24.0f; // Playback speed
                if (instanceFrames[i] >= anim[0].frameCount) instanceFrames[i] = 0;

                renderTransforms[i] = MatrixTranslate((i % 25) * 2.0f, 0, (i / 25) * 2.0f);
                renderTransforms[i].m15 = instanceFrames[i]; // Store frame in m15
                //renderTransforms[i].m15 = 0.0f;
            }
            
            
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(cameras[GetActiveCameraId()].data); 
                //DrawPlane((Vector3){0,-1,0},(Vector2){2000,2000}, GREEN);                                      // Draw a plane XZ
                BeginShaderMode(animShader);

                // --- Step A: Setup the Animation Texture ---
                int animTexLoc = GetShaderLocation(animShader, "uAnimTexture");
                int slotIndex = 1; // We choose slot 1
                SetShaderValue(animShader, animTexLoc, &slotIndex, SHADER_UNIFORM_INT);

                rlActiveTextureSlot(1);      // Switch to slot 1
                rlEnableTexture(bakedTex.id); // Bind our bone data texture

                // --- Step B: Draw ---
                // Raylib will automatically bind model.materials[0].maps[0].texture to slot 0
                DrawMeshInstanced(model.meshes[0], model.materials[0], renderTransforms, MAX_INSTANCES);
                // --- Step C: Cleanup ---
                rlDisableTexture();    // Unbind slot 1
                rlActiveTextureSlot(0); // Switch back to default slot 0

                EndShaderMode();
            
            UpdateLightingUniforms(cameras[0].data, terrainShader, lightDirLoc, lightColorLoc,viewPosLoc);
            //AnimationSystem();
            DrawSystem();    
            DrawLine3D((Vector3){0,0,0},(Vector3){1000,0,0}, RED);
            DrawLine3D((Vector3){0,0,0},(Vector3){0,1000,0}, GREEN);
            DrawLine3D((Vector3){0,0,0},(Vector3){0,0,1000}, BLUE);
            EndMode3D();
            //DrawTexture(bakedTex, 50,50, WHITE);
            DrawTextureEx(bakedTex, (Vector2){50,50},0.0f,10.0f, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
