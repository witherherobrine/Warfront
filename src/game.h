// src/game.h

#ifndef GAME_H
#define GAME_H

// Entity ID is just an index/integer
typedef unsigned int EntityId;

#include "raylib.h"
#include "components.h" // Includes all component structs

#define MAX_ENTITIES 100000

// --- ECS CORE DEFINITIONS ---


// The Component Signature Bitmask
typedef enum {
    COMPONENT_NONE      = 0,
    COMPONENT_TRANSFORM = 1 << 0,
    COMPONENT_RENDER    = 1 << 1,
    COMPONENT_PLAYER    = 1 << 2,
    COMPONENT_CAMERA    = 1 << 3,
    COMPONENT_PARENT    = 1 << 4,
    COMPONENT_TERRAIN_CHUNK   = 1 << 6,
    COMPONENT_SOLDIER   = 1 << 7,
    COMPONENT_ANIMATIONS  = 1 << 7,
} ComponentType;


// --- Global Component Arrays (The Data Storage) ---
// These arrays are sparse; the index corresponds to the EntityId.
// Note: In a larger game, you'd put this in a struct or dynamic map, 
// but for C/raylib simplicity, global arrays are common.

extern TransformComponent transforms[MAX_ENTITIES];
extern RenderComponent renders[MAX_ENTITIES];
extern PlayerComponent players[MAX_ENTITIES];
extern CameraComponent cameras[MAX_ENTITIES];
extern ParentComponent parents[MAX_ENTITIES];
extern TerrainChunkComponent terrainChunks[MAX_ENTITIES];
extern SoldierComponent soldiers[MAX_ENTITIES];
extern AnimationComponent animations[MAX_ENTITIES];

// The core lookup table: Which components does each entity have?
extern unsigned int entityComponentMasks[MAX_ENTITIES];

// Helper variables
extern unsigned int nextFreeEntityId;

// ... declarations for other attach functions

#endif // GAME_H
