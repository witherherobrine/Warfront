// src/systems.c
//#include "systems.h"
#include "game.h"
#include "entities.h"
#include "raymath.h"
#include "systems.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

// --- Rendering System ---

void DrawSystem() {
    // 1. Define the required signature for this system
    const unsigned int requiredMask = COMPONENT_TRANSFORM | COMPONENT_RENDER;

    // 2. Iterate over all active entities
    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        
        // 3. Check if the entity matches the signature
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            
            // 4. Access the component data directly via the EntityId index
            TransformComponent* t = &transforms[id];
            RenderComponent* r = &renders[id];
            DrawModelEx(r->model, 
                t->position, 
                t->rotationAxis, 
                t->rotationAngle, 
                t->scale, 
                WHITE);
        }
    }
}

#define PLAYER_SPEED 15.0 // Units per second

void PlayerInputSystem(float deltaTime) {
    const unsigned int requiredMask = COMPONENT_TRANSFORM | COMPONENT_PLAYER;

    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            
            TransformComponent* t = &transforms[id];
            
            float forwardInput = 0.0f;
            float strafeInput = 0.0f;
            
            // 1. Gather raw input (Forward/Backward and Strafe)
            if (IsKeyDown(KEY_W)) forwardInput += 1.0f; 
            if (IsKeyDown(KEY_S)) forwardInput -= 1.0f; 
            if (IsKeyDown(KEY_A)) strafeInput -= 1.0f; 
            if (IsKeyDown(KEY_D)) strafeInput += 1.0f; 

            // Check if there is any input before proceeding
            if (forwardInput != 0.0f || strafeInput != 0.0f) {
                
                // 2. Get the current Yaw angle (the Soldier's rotation)
                float rotationRad = DEG2RAD * t->rotationAngle;
                
                // 3. Calculate the FORWARD direction vector based on the Yaw
                Vector3 forwardDir;
                // STANDARD FIX: 
                // X: Use sine to get the horizontal deviation.
                forwardDir.x = cosf(rotationRad);  
                // Z: Use negative cosine to point down the -Z axis at 0 degrees.
                forwardDir.z = -sinf(rotationRad); // <-- CRITICAL FIX for forward direction
                forwardDir.y = 0.0f; 
                
                // 4. Calculate the RIGHT direction vector (90 degrees clockwise from forward)
                Vector3 rightDir;
                // RIGHT vector is 90 degrees clockwise from the new forward vector.
                // It should be perpendicular: (z, -x) or (-z, x)
                rightDir.x = sinf(rotationRad);    // X: Use cosine
                rightDir.z = cosf(rotationRad);    // Z: Use sine (positive sign for standard right)
                rightDir.y = 0.0f;

                // 5. Combine and scale the movement vectors
                Vector3 totalMovement = Vector3Zero();
                
                // Forward/Backward movement
                totalMovement = Vector3Add(totalMovement, Vector3Scale(forwardDir, forwardInput));
                
                // Strafe movement
                totalMovement = Vector3Add(totalMovement, Vector3Scale(rightDir, strafeInput));

                // Normalize the combined vector to prevent faster diagonal movement
                totalMovement = Vector3Normalize(totalMovement);
                
                // 6. Apply speed and deltaTime
                Vector3 displacement = Vector3Scale(totalMovement, PLAYER_SPEED * deltaTime);

                // 7. Apply the displacement
                t->position = Vector3Add(t->position, displacement);
                //t->position.y = get_terrain_height(t->position.x, t->position.z)*10+2;
                t->position.y = get_terrain_height(t->position.x, t->position.z)*10+1;
            }
        }
    }
}


void ParentingSystem() {
    const unsigned int requiredMask = COMPONENT_TRANSFORM | COMPONENT_PARENT;

    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            
            ParentComponent* p = &parents[id];
            
            // Check if parent exists and has a transform
            if (p->parentId > 0 && (entityComponentMasks[p->parentId] & COMPONENT_TRANSFORM)) {
                
                TransformComponent* childTransform = &transforms[id];
                TransformComponent* parentTransform = &transforms[p->parentId];
                // Set child position relative to parent
                childTransform->position = Vector3Add(parentTransform->position, p->localOffset);

                //childTransform->position.x = parentTransform->position.x + p->localOffset.x;
                //childTransform->position.y = parentTransform->position.y + p->localOffset.y;
                //childTransform->position.z = parentTransform->position.z + p->localOffset.z;


                // For FPS, the camera (child) rotation needs to be controlled by the mouse,
                // but for simplicity, we can initially just match the YAW.
                childTransform->rotationAngle = parentTransform->rotationAngle;
            }
        }
    }
}


void CameraSystem(float deltaTime) {
    
    const unsigned int requiredMask = COMPONENT_TRANSFORM | COMPONENT_CAMERA;
    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        //printf("%i | %i | %i \n",id, requiredMask, entityComponentMasks[id] & requiredMask);
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            TransformComponent* t = &transforms[id];
            CameraComponent* c = &cameras[id];

            if (!c->isActive) continue;

            // --- 1. Mouse Look Input ---
            Vector2 mouseDelta = GetMouseDelta();
            float sensitivity = 0.1f;

            // Yaw (Horizontal) - Modifies the Soldier's Y-axis rotation
            c->yaw += mouseDelta.x * sensitivity;
            
            // Pitch (Vertical)
            c->pitch -= mouseDelta.y * sensitivity;
            
            // Clamp pitch to prevent flipping the camera
            if (c->pitch > 89.0f) c->pitch = 89.0f;
            if (c->pitch < -89.0f) c->pitch = -89.0f;

            // --- 2. Update Soldier Rotation (CRITICAL FPS STEP) ---
            // The soldier's YAW is driven by the camera's YAW
            // If the camera has a parent (the soldier), update the parent's Y-rotation
            if ((entityComponentMasks[id] & COMPONENT_PARENT) && parents[id].parentId > 0) {
                transforms[parents[id].parentId].rotationAngle = -c->yaw;
            } else {
                 // If not attached, update the camera's transform directly
                 t->rotationAngle = c->yaw;
            }

            // --- 3. Recalculate Camera Vectors ---
            // Calculate forward, right, and up vectors based on yaw/pitch
            // (Using raylib/raymath for this is complex, but the goal is to set c->data.target)
            
            Vector3 forward;
            forward.x = cosf(DEG2RAD * c->yaw) * cosf(DEG2RAD * c->pitch);
            forward.y = sinf(DEG2RAD * c->pitch);
            forward.z = sinf(DEG2RAD * c->yaw) * cosf(DEG2RAD * c->pitch);
            forward = Vector3Normalize(forward);

            c->data.position = t->position;
            c->data.target = Vector3Add(t->position, forward);
            c->data.up = (Vector3){0.0f, 1.0f, 0.0f}; 
        }
    }
}

void UpdateLightingUniforms(Camera camera, Shader terrainShader, int lightDirLoc, int lightColorLoc, int viewPosLoc) {
    // Sun Light Direction (Slightly angled down, pointing toward the origin)
    // If the light is moving, this needs to be calculated dynamically.
    Vector3 sunDirection = { -0.6f,0.4f,0.0f }; // X-Z axis, pointing down Y
    //sunDirection = Vector3Normalize(sunDirection);
    
    // Convert to a C array format for Raylib
    float lightDir[] = { sunDirection.x, sunDirection.y, sunDirection.z };
    float lightColor[] = { 1.0f, 1.0f, 1.0f }; // White light
    
    // 1. Update Global Light Properties (Once per frame)
    SetShaderValue(terrainShader, lightDirLoc, lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, lightColorLoc, lightColor, SHADER_UNIFORM_VEC3);

    // 2. Update Camera Position (Crucial for Specular Highlights)
    //float viewPos[] = { camera.position.x, camera.position.y, camera.position.z };
    int pos[] = {400,-1000,0};
    SetShaderValue(terrainShader, viewPosLoc,pos, SHADER_UNIFORM_VEC3);
}


// src/systems.c

// System to manage which chunks are loaded/unloaded
void ViewDistanceManagerSystem(Texture t, Shader s) {
    
    
    const int CHUNKS_PER_SIDE = 200;
    const float CHUNK_WORLD_SIZE = 64.0f; // Each chunk is 64x64 units
    const int CHUNK_RESOLUTION = 9;     // 129 vertices for a 128x128 grid
    
    
    // 1. Get Player Position (Assume player is EntityId 1)
    const EntityId playerId = 2; 
    PlayerComponent* player = &players[playerId];
    TransformComponent* t_player = &transforms[playerId];

    // Determine the chunk the player is currently in
    // (You'll need to calculate this based on world position and CHUNK_WORLD_SIZE)
    int playerChunkX = (int)(t_player->position.x / CHUNK_WORLD_SIZE);
    int playerChunkZ = (int)(t_player->position.z / CHUNK_WORLD_SIZE);
    
    // Update player component for consistency (optional)
    player->currentChunkX = playerChunkX;
    player->currentChunkZ = playerChunkZ;

    const int VIEW_RADIUS = 65; 

    // 2. Iterate through ALL Terrain Chunk Entities (40,000 entities)
    // NOTE: This array loop is fast because we only perform simple integer math.
    const unsigned int requiredMask = COMPONENT_TERRAIN_CHUNK | COMPONENT_RENDER;
    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            TerrainChunkComponent* chunk = &terrainChunks[id];
            RenderComponent* r = &renders[id];

            // Calculate distance in chunk grid units
            int dx = abs(chunk->gridX - playerChunkX);
            int dz = abs(chunk->gridZ - playerChunkZ);
            
            // Manhattan distance for simplicity in a square view radius
            bool isWithinView = (dx <= VIEW_RADIUS) && (dz <= VIEW_RADIUS);

            // --- CULLING LOGIC ---
            
            if (isWithinView && !chunk->isLoaded) {
                // Action: Chunk should be visible but is NOT loaded. --> LOAD

                Mesh tMesh = GenMeshPlane(CHUNK_WORLD_SIZE,CHUNK_WORLD_SIZE, CHUNK_RESOLUTION,CHUNK_RESOLUTION); 

                fnl_state noise = fnlCreateState();
                noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    
                for (int i = 0; i < tMesh.vertexCount; i++) {
                    float yVal = get_terrain_height(tMesh.vertices[i*3] + (chunk->gridX*CHUNK_WORLD_SIZE), tMesh.vertices[i*3+2] + (chunk->gridZ*CHUNK_WORLD_SIZE))*10;
                    tMesh.vertices[i * 3 + 1] = yVal; // Modify the y-value
                }
                RecalculateMeshNormals(&tMesh);
                UpdateMeshBuffer(tMesh, 0, tMesh.vertices, tMesh.vertexCount * 3 * sizeof(float), 0);
                UpdateMeshBuffer(tMesh, 2, tMesh.normals, tMesh.vertexCount * 3 * sizeof(float), 0);

                r->model = LoadModelFromMesh(tMesh);
                r->model.materials[0].shader = s;
                r->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = t;
                
                chunk->isLoaded = true;
                r->isVisible = true; 
            }
            else if (!isWithinView && chunk->isLoaded) {
                // Action: Chunk is outside view distance but IS loaded. --> UNLOAD
                
                // Unload VRAM data and free CPU data (Except the O(1) cache!)
                UnloadModel(r->model); // Frees VRAM
                r->model.meshCount = 0; // Mark as empty/unloaded
                
                chunk->isLoaded = false;
                r->isVisible = false;
            }
            // If isWithinView and isLoaded, do nothing. (It's visible and ready)
        }
    }
}
void temporaryAIMoveSystem(void){
    
    const unsigned int requiredMask = COMPONENT_TRANSFORM | COMPONENT_SOLDIER;
    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            TransformComponent* t = &transforms[id];
            SoldierComponent* s = &soldiers[id];
            
           
            float dx = s->destination.x - t->position.x;
            float dy = s->destination.z - t->position.z;
            
            if(Vector2DistanceSqr((Vector2){s->destination.x,s->destination.z}, (Vector2){t->position.x, t->position.z}) < 25){
                s-> destination = (Vector3){GetRandomValue(1,200),0,GetRandomValue(1,200)};
                continue;
            }
            
            float length = sqrt((dx*dx)+(dy*dy));
            dx /= length;
            dy /= length;
            //dx *=.1f;
            //dy *=.1f;
            //.03f
            Vector3 velocity = Vector3Scale((Vector3){dx,0,dy},.1f);
            t->position = Vector3Add(t->position, velocity);
           
           
            t->position.y = get_terrain_height(t->position.x, t->position.z)*10+2;
            
            Vector3 direction = {s->destination.x - t->position.x, 0.0f, s->destination.z - t->position.z};
            float ang = atan2f(direction.x, direction.z) * (180/M_PI) - 90;
            t->rotationAngle = ang;
        }
    }
}
void AnimationSystem(void){
    const unsigned int requiredMask = COMPONENT_RENDER | COMPONENT_ANIMATIONS;
    for (EntityId id = 1; id < nextFreeEntityId; id++) {
        if ((entityComponentMasks[id] & requiredMask) == requiredMask) {
            
            //TransformComponent* t = &transforms[id];
            RenderComponent* model = &renders[id];
            AnimationComponent* anim = &animations[id];
            UpdateModelAnimation(model->model, anim->animations[anim->animIndex], anim->animFrame);
            anim->animFrame++;
            
            if(anim->animFrame >= anim->animations[anim->animIndex].frameCount){
                anim-> animFrame = 0;
            }
        }
    }
}


















