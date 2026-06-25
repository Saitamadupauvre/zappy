
#ifndef WORLD_H
    #define WORLD_H

#include "config.h"

    #define RESPAWN_INTERVAL 20

typedef enum {
    FOOD,
    LINEMATE,
    DERAUMERE,
    SIBUR,
    MENDIANE,
    PHIRAS,
    THYSTAME,
    RESOURCE_COUNT
} Ressources;

typedef struct map_tile_s {
    int ressources[RESOURCE_COUNT];
    int player_count;
    int egg_count;
} map_tile_t;

typedef struct world_map_s {
    map_tile_t *tiles;
    int w;
    int h;
} world_map_t;


int init_world_map(prog_cfg_t *prog_cfg);

void refill_world_resources(world_map_t *world_map);

void print_world_map(world_map_t *world_map);

#endif
