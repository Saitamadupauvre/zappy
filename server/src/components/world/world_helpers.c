
#include "arena.h"
#include "config.h"
#include "world.h"

#include <stdio.h>


static const char *resource_names[RESOURCE_COUNT] = {
    [FOOD]      = "food",
    [LINEMATE]  = "linemate",
    [DERAUMERE] = "deraumere",
    [SIBUR]     = "sibur",
    [MENDIANE]  = "mendiane",
    [PHIRAS]    = "phiras",
    [THYSTAME]  = "thystame",
};

void print_world_map(world_map_t *world_map)
{
    size_t tile_count = world_map->w * world_map->h;

    for (size_t i = 0; i < tile_count; i++) {
        map_tile_t *tile = &world_map->tiles[i];
        int x = i % world_map->w;
        int y = i / world_map->w;
        printf("Tile [%d,%d] | players: %d | eggs: %d | ",
            x, y, tile->player_count, tile->egg_count);
        for (int r = 0; r < RESOURCE_COUNT; r++) {
            if (tile->ressources[r] > 0)
                printf("%s: %d ", resource_names[r], tile->ressources[r]);
        }
        printf("\n");
    }
}
