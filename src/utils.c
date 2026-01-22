

#include "utils.h"

fnl_state global_terrain_noise; 
float get_terrain_height(float x, float z) {
    return fnlGetNoise2D(&global_terrain_noise, x, z);
}

void init_game_world(void) {
    global_terrain_noise = fnlCreateState();
    global_terrain_noise.noise_type = FNL_NOISE_PERLIN; 
    global_terrain_noise.seed = 42; 
    global_terrain_noise.frequency = 0.015f; 
}
