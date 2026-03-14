

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
        //attach_render(e, model);
        //attach_animations(e, anim);
        attach_soldier(e);
    }
}
void PrintMatrix(const char* label, Matrix mat) {
    printf("%s:\n", label);
    printf("  [%7.3f %7.3f %7.3f %7.3f]\n", mat.m0, mat.m4, mat.m8,  mat.m12);
    printf("  [%7.3f %7.3f %7.3f %7.3f]\n", mat.m1, mat.m5, mat.m9,  mat.m13);
    printf("  [%7.3f %7.3f %7.3f %7.3f]\n", mat.m2, mat.m6, mat.m10, mat.m14);
    printf("  [%7.3f %7.3f %7.3f %7.3f]\n", mat.m3, mat.m7, mat.m11, mat.m15);
    printf("\n");
}


Matrix* BakeAnimationFrames(Model model, ModelAnimation anim, int* outMatrixCount) {
    int frameCount = anim.frameCount;
    int boneCount = anim.boneCount;
    *outMatrixCount = frameCount * boneCount;

    Matrix* bakedMatrices = (Matrix*)RL_MALLOC(frameCount * boneCount * sizeof(Matrix));
    Matrix* worldTransforms = (Matrix*)RL_MALLOC(boneCount * sizeof(Matrix));
    Matrix* invGlobalBindPose = (Matrix*)RL_MALLOC(boneCount * sizeof(Matrix));

    // --- STEP 1: Pre-calculate Global Inverse Bind Poses using Frame 0 ---
    // This ensures that the Bind Pose and Animation use the exact same basis.
    for (int i = 0; i < boneCount; i++) {
        Transform basePose = anim.framePoses[0][i]; 
        
        Matrix matScale = MatrixScale(basePose.scale.x, basePose.scale.y, basePose.scale.z);
        Matrix matRotation = QuaternionToMatrix(basePose.rotation);
        Matrix matTranslation = MatrixTranslate(basePose.translation.x, basePose.translation.y, basePose.translation.z);

        // Standard TRS
        Matrix localBind = MatrixMultiply(matScale, matRotation);
        localBind = MatrixMultiply(matTranslation, localBind);

        int parentIndex = model.bones[i].parent;
        if (parentIndex == -1) {
            worldTransforms[i] = localBind;
        } else {
            worldTransforms[i] = MatrixMultiply(worldTransforms[parentIndex], localBind);
        }
        
        if (i == 1) {
            printf("--- DEBUG BONE %d", i);
            PrintMatrix(":--LOCAL--:", localBind);
            PrintMatrix(":-WORLD--:", worldTransforms[i]);
        }
        
        
        invGlobalBindPose[i] = MatrixInvert(worldTransforms[i]);
    }

    // --- STEP 2: Bake Animation Frames ---
    for (int f = 0; f < frameCount; f++) {
        for (int b = 0; b < boneCount; b++) {
            Transform pose = anim.framePoses[f][b];
            
            Quaternion animRot = pose.rotation;
            Quaternion correctedRot = { animRot.x, animRot.z, -animRot.y, animRot.w };
            
            Matrix matScale = MatrixScale(pose.scale.x, pose.scale.y, pose.scale.z);
            Matrix matRotation = QuaternionToMatrix(pose.rotation);
            Matrix matTranslation = MatrixTranslate(pose.translation.x, pose.translation.y, pose.translation.z);
            
            // Standard TRS: Translation * Rotation * Scale
            Matrix localMat = MatrixMultiply(matRotation, matScale); 
            localMat = MatrixMultiply(matTranslation, localMat);
            
            int parentIndex = model.bones[b].parent;
            if (parentIndex == -1) {
                worldTransforms[b] = localMat;
            } else {
                worldTransforms[b] = MatrixMultiply(worldTransforms[parentIndex], localMat);
            }

            int index = f * boneCount + b;
            
            // Calculate skinning matrix
            Matrix result = MatrixMultiply(invGlobalBindPose[b], worldTransforms[b]);
            
            bakedMatrices[index] = result;
            
            // Debugging Bone 1 (The lower prism)
            //if (b == 1 && f < 11) {
                //printf("--- DEBUG BONE %d | FRAME %d ---\n", b, f);
                //PrintMatrix("World Transform (Animated)", worldTransforms[b]);
                //PrintMatrix("Inverse Global Bind Pose", invGlobalBindPose[b]);
                //PrintMatrix("FINAL BAKED (CORRECTED)", bakedMatrices[index]);
            //}
        }
    }

    RL_FREE(worldTransforms);
    RL_FREE(invGlobalBindPose);
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


void LogModelBoneNames(Model model) {
    if (model.boneCount > 0) {
        TraceLog(LOG_INFO, "MODEL: Total bones found: %d", model.boneCount);
        
        for (int i = 0; i < model.boneCount; i++) {
            // Using %02d to keep the indices aligned in the console
            TraceLog(LOG_INFO, "  [Bone %02d] Name: %s (Parent Index: %d)", 
                     i, model.bones[i].name, model.bones[i].parent);
        }
    } else {
        TraceLog(LOG_WARNING, "MODEL: No bone data found in this model.");
    }
}


void LogDebugAnim(ModelAnimation* anim) {
    TraceLog(LOG_INFO, "ANIM - Name: %s", anim->name); 
    TraceLog(LOG_INFO, "ANIM - BoneCount: %i", anim->boneCount);
    TraceLog(LOG_INFO, "ANIM - FrameCount: %i", anim->frameCount); 
    
    // The bone you want to track (e.g., UpperArm_L)
    int boneTrackIndex = 1;

    for(int i = 0; i < anim->frameCount; i++) {
        // 1. Get the array of transforms for the current frame
        Transform *frameTransforms = anim->framePoses[i];
        
        // 2. Access the specific bone within that frame
        Vector3 pos = frameTransforms[boneTrackIndex].translation;
        Quaternion rot = frameTransforms[boneTrackIndex].rotation;
        Vector3 scl = frameTransforms[boneTrackIndex].scale;

        TraceLog(LOG_INFO, "FRAME[%02i] BONE[%i] - Pos: (%.3f, %.3f, %.3f)", 
                 i, boneTrackIndex, pos.x, pos.y, pos.z);
        
        TraceLog(LOG_INFO, "FRAME[%02i] BONE[%i] - Rot: (%.3f, %.3f, %.3f, %.3f)", 
                 i, boneTrackIndex, rot.w, rot.x, rot.y, rot.z);
    }
}

void CheckBoneInfluence(Mesh mesh, int boneIndex) {
    int affectedVertices = 0;
    
    // Each vertex has 4 bone IDs and 4 weights (standard raylib/gltf/m3d)
    for (int i = 0; i < mesh.vertexCount; i++) {
        bool usesBone = false;
        
        // Bone IDs are typically stored as unsigned chars or floats in mesh.boneIds
        // We check all 4 potential slots for this vertex
        if (mesh.boneIds[i * 4 + 0] == boneIndex || 
            mesh.boneIds[i * 4 + 1] == boneIndex || 
            mesh.boneIds[i * 4 + 2] == boneIndex || 
            mesh.boneIds[i * 4 + 3] == boneIndex) {
            affectedVertices++;
        }
    }

    if (affectedVertices > 0) {
        TraceLog(LOG_WARNING, "BONE[%d] is active! It influences %d vertices.\n", boneIndex, affectedVertices);
    } else {
        TraceLog(LOG_INFO, "BONE[%d] is a ghost; no vertices are attached to it.\n", boneIndex);
    }
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
    
    Model model = LoadModel("../res/models/person/TESTECOOBE.m3d");
    model.boneCount--;
    LogModelBoneNames(model);
    CheckBoneInfluence(model.meshes[0], 1);
    
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
    ModelAnimation* anim = LoadModelAnimations("../res/models/person/TESTECOOBE.m3d", &animsCount);
    anim->boneCount--;

    if (anim->boneCount == 17 && strcmp(anim->bones[16].name, "NO BONE") == 0) {
        anim->boneCount = 16; 
        printf("reset animbonecount\n");
    }
    
    
    //LogDebugAnim(anim);


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
        model.meshes[0].vertexCount * 4 * sizeof(unsigned char), // FIXED: Use unsigned char
        false
    );
    rlEnableVertexAttribute(boneIdsLoc);
    rlSetVertexAttribute(boneIdsLoc, 4, 0x1401, false, 0, 0); // 0x1401 is GL_UNSIGNED_BYTE
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
    
    int MAX_INSTANCES = 3;
    // 6. Setup Instances
    Matrix renderTransforms[MAX_INSTANCES];
    int instanceFrames[MAX_INSTANCES];
    for (int i = 0; i < MAX_INSTANCES; i++) {
        //instanceFrames[i] = (int)GetRandomValue(0, anim[0].frameCount - 1);
        instanceFrames[i] = 0;
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
        if(tframe > 20){
            tframe = 0;
            }
            
            for (int i = 0; i < MAX_INSTANCES; i++) {
                instanceFrames[i] += GetFrameTime() * 60.0f; // Playback speed
                if (instanceFrames[i] >= anim[0].frameCount) instanceFrames[i] = 0;

                renderTransforms[i] = MatrixTranslate((i % 25) * 5.0f, 5, (i / 25) * 5.0f);
                renderTransforms[i].m15 = instanceFrames[i]; // Store frame in m15
                
                //renderTransforms[i].m15 = 20;
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
            DrawLine3D((Vector3){0,0,0},(Vector3){1000,0,0}, RED); //X
            DrawLine3D((Vector3){0,0,0},(Vector3){0,1000,0}, GREEN); //Y
            DrawLine3D((Vector3){0,0,0},(Vector3){0,0,1000}, BLUE); //Z
            EndMode3D();
            //DrawTexture(bakedTex, 50,50, WHITE);
            DrawTextureEx(bakedTex, (Vector2){50,50},0.0f,10.0f, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
