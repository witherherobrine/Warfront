

#ifndef UTILS_H
#define UTILS_H
#include "FastNoiseLite.h"

extern fnl_state global_terrain_noise; 
float get_terrain_height(float x, float z);
void init_game_world(void);


#endif
