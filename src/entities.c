// src/entities.c
#include "game.h"
#include "entities.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include <stdlib.h>

#include <stdio.h>
#include <string.h> // For memset

// --- Global Component Array Definitions (Must match externs in game.h) ---
TransformComponent transforms[MAX_ENTITIES];
RenderComponent renders[MAX_ENTITIES];
PlayerComponent players[MAX_ENTITIES];
CameraComponent cameras[MAX_ENTITIES];
ParentComponent parents[MAX_ENTITIES];
TerrainChunkComponent terrainChunks[MAX_ENTITIES];
SoldierComponent soldiers[MAX_ENTITIES];
AnimationComponent animations[MAX_ENTITIES];

unsigned int entityComponentMasks[MAX_ENTITIES] = {0};
unsigned int nextFreeEntityId = 1;

EntityId activeCameraId = 0; // Initialize to 0 (invalid ID)

// --- Core Entity Management ---

EntityId create_entity() {
    if (nextFreeEntityId >= MAX_ENTITIES) return 0; // Out of IDs
    
    EntityId newId = nextFreeEntityId++;
    entityComponentMasks[newId] = COMPONENT_NONE;

    // Optional: Zero out component memory for safety
    // For large games, this is often skipped for performance, 
    // but useful during development.
    //memset(&transforms[newId], 0, sizeof(TransformComponent));
    // ... memset other component structs

    return newId;
}

void destroy_entity(EntityId id) {
    if (id == 0 || id >= nextFreeEntityId) return;

    // In a simple ECS, destroying means clearing the mask.
    // Memory can be reused later. For now, we just de-activate it.
    entityComponentMasks[id] = COMPONENT_NONE; 

    // Note: You should handle raylib unloading (e.g., UnloadModel) here
    // if the entity had a RenderComponent. This is often done in a dedicated 
    // "CleanupSystem."
}

// --- Component Attachment Functions ---


void attach_transform(EntityId id, Vector3 pos, Vector3 rotAx, float rotAng, Vector3 scale) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    
    transforms[id].position = pos;
    transforms[id].rotationAxis = rotAx;
    transforms[id].rotationAngle = rotAng;
    transforms[id].scale = scale;
    
    entityComponentMasks[id] |= COMPONENT_TRANSFORM;
}

void attach_render(EntityId id, Model model) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    renders[id].model = model;
    entityComponentMasks[id] |= COMPONENT_RENDER;
}

void attach_player_tag(EntityId id) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    players[id].sensitivity = 0.05f; 
    entityComponentMasks[id] |= COMPONENT_PLAYER;
}

void attach_camera(EntityId id, float fovY, float pitch, float yaw, bool isActive) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    
    CameraComponent* c = &cameras[id];

    // Initialize the raylib Camera3D struct with basic values
    c->data = (Camera3D){ 
        (Vector3){ 0.0f, 1.8f, 0.0f },    // Initial position (will be overwritten by ParentingSystem)
        (Vector3){ 0.0f, 1.8f, -1.0f },   // Initial target
        (Vector3){ 0.0f, 1.0f, 0.0f },    // Up vector
        fovY,                             // Field-of-View
        CAMERA_PERSPECTIVE                // Projection mode
    };

    c->yaw = yaw;
    c->pitch = pitch;
    c->distanceOffset = 0.0f; // FPS view
    c->isActive = isActive;
    
    entityComponentMasks[id] |= COMPONENT_CAMERA;
}

void attach_parent(EntityId id, EntityId parentId, Vector3 localOffset) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    
    ParentComponent* p = &parents[id];

    p->parentId = parentId;
    p->localOffset = localOffset;
    
    entityComponentMasks[id] |= COMPONENT_PARENT;
}
//void attach_terrain_tag(EntityId id) {
    //if (id == 0 || id >= MAX_ENTITIES) return;
    
    //entityComponentMasks[id] |= COMPONENT_TERRAIN;
//}
void attach_terrain_chunk(EntityId id, int gx, int gz, float worldSize, float resolution){
    if (id == 0 || id >= MAX_ENTITIES) return;
    entityComponentMasks[id] |= COMPONENT_TERRAIN_CHUNK;
    
    TerrainChunkComponent* c = &terrainChunks[id];
    c->gridX = gx;
    c->gridZ = gz;
    c->worldSize = worldSize;
    c->resolution = resolution;
    c->isMeshDirty = true; // Needs initial mesh creation
    c->isLoaded = true;
    
}
void attach_soldier(EntityId id){
    if (id == 0 || id >= MAX_ENTITIES) return;
    entityComponentMasks[id] |= COMPONENT_SOLDIER;
    soldiers[id].health = 100;
    soldiers[id].baseSpeed = 1;
    
}

void attach_animations(EntityId id, ModelAnimation* anims) {
    if (id == 0 || id >= MAX_ENTITIES) return;
    animations[id].animations = anims;
    animations[id].animFrame = 0;
    animations[id].animIndex = 0;
    entityComponentMasks[id] |= COMPONENT_RENDER;
}











// Function to recalculate mesh normals after vertex modification
void RecalculateMeshNormals(Mesh *mesh)
{
    // A temporary array to accumulate normals for each vertex
    // Must be initialized to zero
    Vector3 *newNormals = (Vector3*)RL_MALLOC(mesh->vertexCount * sizeof(Vector3));
    memset(newNormals, 0, mesh->vertexCount * sizeof(Vector3));

    // Pointers for easier access
    Vector3 *vertices = (Vector3*)mesh->vertices;
    unsigned short *indices = mesh->indices;
    
    // --- 1. Iterate through all triangles (3 indices per triangle) ---
    for (int i = 0; i < mesh->triangleCount; i++)
    {
        // Get the indices of the three vertices for the current triangle
        int i1 = indices[i * 3];
        int i2 = indices[i * 3 + 1];
        int i3 = indices[i * 3 + 2];
        
        // Get the vertex positions
        Vector3 v1 = vertices[i1];
        Vector3 v2 = vertices[i2];
        Vector3 v3 = vertices[i3];

        // Calculate two edges of the triangle
        Vector3 edge1 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
        Vector3 edge2 = { v3.x - v1.x, v3.y - v1.y, v3.z - v1.z };

        // Calculate the face normal using the Cross Product
        Vector3 faceNormal = Vector3CrossProduct(edge1, edge2);
        // NOTE: We don't normalize the face normal yet; we want its magnitude to 
        // contribute to the vertex normal average based on triangle area.
        
        // Accumulate the face normal to the three vertices
        newNormals[i1].x += faceNormal.x;
        newNormals[i1].y += faceNormal.y;
        newNormals[i1].z += faceNormal.z;

        newNormals[i2].x += faceNormal.x;
        newNormals[i2].y += faceNormal.y;
        newNormals[i2].z += faceNormal.z;

        newNormals[i3].x += faceNormal.x;
        newNormals[i3].y += faceNormal.y;
        newNormals[i3].z += faceNormal.z;
    }
    
    // --- 2. Normalize the accumulated normals and copy to mesh buffer ---
    
    // Check if mesh->normals is allocated, if not, allocate it
    if (mesh->normals == NULL)
    {
        mesh->normals = (float*)RL_MALLOC(mesh->vertexCount * 3 * sizeof(float));
    }

    for (int i = 0; i < mesh->vertexCount; i++)
    {
        // Normalize the accumulated vertex normal
        Vector3 finalNormal = Vector3Normalize(newNormals[i]);
        
        // Copy to the mesh->normals float array (x, y, z, x, y, z, ...)
        mesh->normals[i * 3]     = finalNormal.x;
        mesh->normals[i * 3 + 1] = finalNormal.y;
        mesh->normals[i * 3 + 2] = finalNormal.z;
    }

    RL_FREE(newNormals);
}



void TerrainSetupSystem(Shader shader) {
    const int CHUNKS_PER_SIDE = 5;
    const float CHUNK_WORLD_SIZE = 64.0f; // Each chunk is 64x64 units
    const int CHUNK_RESOLUTION = 9;     // 129 vertices for a 128x128 grid
    
    Texture2D groundTexture = LoadTexture("../res/textures/grass.png");
    
    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    
    // Total number of entities to create: 8 * 8 = 64
    for (int gz = 0; gz < CHUNKS_PER_SIDE; gz++) {
        for (int gx = 0; gx < CHUNKS_PER_SIDE; gx++) {
            
            Mesh tMesh = GenMeshPlane(CHUNK_WORLD_SIZE,CHUNK_WORLD_SIZE, CHUNK_RESOLUTION,CHUNK_RESOLUTION); 
            
            for (int i = 0; i < tMesh.vertexCount; i++) {
                //float yVal = fnlGetNoise2D(&noise, tMesh.vertices[i*3] + (gx*CHUNK_WORLD_SIZE), tMesh.vertices[i*3+2] + (gz*CHUNK_WORLD_SIZE))*10;
                float yVal = get_terrain_height(tMesh.vertices[i*3] + (gx*CHUNK_WORLD_SIZE), tMesh.vertices[i*3+2] + (gz*CHUNK_WORLD_SIZE))*10;

                tMesh.vertices[i * 3 + 1] = yVal; // Modify the y-value
                //tMesh.vertices[i * 3 + 1] = 0;
            }
            RecalculateMeshNormals(&tMesh);
            UpdateMeshBuffer(tMesh, 0, tMesh.vertices, tMesh.vertexCount * 3 * sizeof(float), 0);
            UpdateMeshBuffer(tMesh, 2, tMesh.normals, tMesh.vertexCount * 3 * sizeof(float), 0);
            
            EntityId chunkId = create_entity(); 
            
            Model terrainModel = LoadModelFromMesh(tMesh);
            terrainModel.materials[0].shader = shader;
            terrainModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture;
            
            float worldX = (float)gx * CHUNK_WORLD_SIZE;
            float worldZ = (float)gz * CHUNK_WORLD_SIZE;
            
            attach_transform(chunkId, 
                (Vector3){worldX,0,worldZ}, // position
                (Vector3){1.0f, 0.0f, 0.0f}, // rotationAxis (Y-axis)
                0.0f,                         // rotationAngle (0 degrees)
                (Vector3){1,1,1}); // scale
            
            attach_terrain_chunk(chunkId, gx, gz, CHUNK_WORLD_SIZE, CHUNK_RESOLUTION);
            attach_render(chunkId, terrainModel);

        }
    }
}







EntityId GetActiveCameraId(void) {
    return activeCameraId;
}

// Setter: Sets the new active camera. Performs a basic check.
void SetActiveCamera(EntityId id) {
    // Check if the ID is valid and has a CameraComponent
    if (id > 0 && (entityComponentMasks[id] & COMPONENT_CAMERA)) {
        // You could also iterate and set all other camera entities to isActive=false here
        activeCameraId = id;
        cameras[id].isActive = true;
    } else {
        // Optional: Reset if the ID is invalid
        // activeCameraId = 0;
    }
}















