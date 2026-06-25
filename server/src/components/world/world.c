
#include "arena.h"
#include "config.h"
#include "world.h"
#include "errors.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const float densities[RESOURCE_COUNT] = {
    [FOOD]      = 0.5f,
    [LINEMATE]  = 0.3f,
    [DERAUMERE] = 0.15f,
    [SIBUR]     = 0.1f,
    [MENDIANE]  = 0.1f,
    [PHIRAS]    = 0.08f,
    [THYSTAME]  = 0.05f,
};

static int resource_target(const world_map_t *world_map, int t)
{
    int count = (int)(world_map->w * world_map->h * densities[t]);

    if (count < 1) { count = 1; }
    return count;
}

static int resource_total(const world_map_t *world_map, int t)
{
    int total = 0;
    int tile_count = world_map->w * world_map->h;

    for (int i = 0; i < tile_count; i++) {
        total += world_map->tiles[i].ressources[t];
    }
    return total;
}

static void scatter_resource(world_map_t *world_map, int t, int count)
{
    for (int o = 0; o < count; o++) {
        int tile_x = rand() % world_map->w;
        int tile_y = rand() % world_map->h;
        world_map->tiles[tile_y * world_map->w + tile_x].ressources[t]++;
    }
}

void refill_world_resources(world_map_t *world_map)
{
    if (world_map == NULL || world_map->tiles == NULL ||
        world_map->w <= 0 || world_map->h <= 0) {
        return;
    }

    for (int t = 0; t < RESOURCE_COUNT; t++) {
        int deficit = resource_target(world_map, t) - resource_total(world_map, t);
        if (deficit > 0) {
            scatter_resource(world_map, t, deficit);
        }
    }
}

int init_world_map(prog_cfg_t *prog_cfg)
{
    prog_cfg->world_map = alloc_mem_arena(&prog_cfg->perm_mem_arena, sizeof(world_map_t));
    if (prog_cfg->world_map == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return -1;
    }

    prog_cfg->world_map->w = prog_cfg->w;
    prog_cfg->world_map->h = prog_cfg->h;
    int tile_count = prog_cfg->world_map->w * prog_cfg->world_map->h;
    size_t world_size = sizeof(map_tile_t) * tile_count;

    prog_cfg->world_map->tiles = alloc_mem_arena(&prog_cfg->perm_mem_arena, world_size);
    if (prog_cfg->world_map->tiles == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return -1;
    }
    memset(prog_cfg->world_map->tiles, 0, world_size);

    refill_world_resources(prog_cfg->world_map);
    return 0;
}
