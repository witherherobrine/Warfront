// src/components.h
#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"
#include <stdbool.h>

// --- Transform Component ---
typedef struct {
    Vector3 position;
    Vector3 rotationAxis;   // The vector to rotate around (e.g., {0, 1, 0} for Y-axis rotation)
    float rotationAngle;    // The angle in degrees
    Vector3 scale;
} TransformComponent;

// --- Render Component ---
typedef struct {
    Model model;
    bool isVisible;         // Frustum Culling flag
} RenderComponent;

// --- Player Component ---
typedef struct {
    float sensitivity;
    int currentChunkX; 
    int currentChunkZ;
} PlayerComponent; 

// --- Camera Component ---
typedef struct {
    Camera3D data;
    bool isActive; // Only one camera should be active/rendered at a time
    float yaw;     // Horizontal rotation (around Y-axis)
    float pitch;   // Vertical rotation (up/down)
    float distanceOffset; // Distance from the target (e.g., 0 for FPS, >0 for 3rd-person)
} CameraComponent;

// --- Parent/Child Component ---
typedef struct {
    EntityId parentId; // The ID of the entity this one is attached to
    Vector3 localOffset; // Position relative to the parent (e.g., {0, 1.8, 0} for head height)
} ParentComponent;

typedef struct {
    int gridX;              // Chunk's X coordinate in the 8x8 grid (0-7)
    int gridZ;              // Chunk's Z coordinate in the 8x8 grid (0-7)
    float worldSize;        // Physical dimension of the chunk (e.g., 64.0 units)
    int resolution;         // Vertices per side (e.g., 129 for 128x128 grid)
    float* heightMapData;   // Caches the Y height values (for O(1) collision/snapping)
    bool isMeshDirty;       // Flag: True if mesh needs to be rebuilt (e.g., LOD change)
    bool isLoaded;      // True if the mesh/model is currently in VRAM
} TerrainChunkComponent;

typedef struct {
    float health;
    float baseSpeed;
    Vector3 destination;
    
} SoldierComponent;

// --- Render Component ---
typedef struct {
    ModelAnimation* animations;
    int animIndex; //what animation it is
    int animFrame; //frame
} AnimationComponent;









#endif // COMPONENTS_H
