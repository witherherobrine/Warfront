#include <stdint.h>
#include <stdbool.h>

typedef uint16_t Entity;
#define MAX_ENTITIES 65536

// Component Mask: Supports up to 32 unique components
typedef uint32_t ComponentMask; 

// Define unique IDs for each component type
typedef enum {
    // Core Components (0-7)
    COMP_POSITION = 0,
    COMP_VELOCITY,
    COMP_SPRITE,
    COMP_COLLIDER,
    
    // Soldier/Game Specific Components (8+)
    COMP_SOLDIER_STATS, 
    COMP_MOVEMENT_TARGET,
    
    // Always last, ensure this is <= 32
    COMP_COUNT 
} ComponentType;

// --- Entity Manager Data ---

// Central storage for the masks
ComponentMask entity_masks[MAX_ENTITIES]; 

// Array to manage available/used Entity IDs (Freelist or simple counter)
// For now, let's just track the next available ID
Entity next_entity_id = 1; // 0 can often be reserved for an invalid entity
