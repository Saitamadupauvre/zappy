
#include "arena.h"
#include "server.h"
#include "commands.h"
#include "world.h"
#include "math.h"
#include "event_sink.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>


typedef struct dir_vec_s {
    int fdx;
    int fdy;
    int rdx;
    int rdy;
} dir_vec_t;

static int wrap(int v, int max) {
    return ((v % max) + max) % max;
}

static void append_str(char *buf, size_t *pos, const char *str)
{
    size_t len = strlen(str);
    memcpy(buf + *pos, str, len);
    *pos += len;
}

static size_t emit_tile(char *buf, size_t pos, map_tile_t *tile, bool *first)
{
    size_t start = pos;

    for (int p = 0; p < tile->player_count; p++) {
        if (!*first) {
            if (buf) append_str(buf, &pos, " ");
            else pos += 1;
        }
        if (buf) append_str(buf, &pos, "player");
        else pos += strlen("player");
        *first = false;
    }
    for (int r = 0; r < RESOURCE_COUNT; r++) {
        const char *name = ressources_to_str((Ressources)r);
        for (int cnt = 0; cnt < tile->ressources[r]; cnt++) {
            if (!*first) {
                if (buf) append_str(buf, &pos, " ");
                else pos += 1;
            }
            if (buf) append_str(buf, &pos, name);
            else pos += strlen(name);
            *first = false;
        }
    }

    return pos - start;
}

static void get_size_per_tile(server_t *server, client_t *client, size_t *size, dir_vec_t *dirs)
{
    bool first_tile = true;

    int vision = client->lvl;
    int w = server->prog_cfg->w;
    int h = server->prog_cfg->h;

    for (int row = 0; row <= vision; row++) {
        for (int col = -row; col <= row; col++) {
            if (!first_tile) *size += 2; // ", "
            int tx = wrap(client->pos.x + row * dirs->fdx + col * dirs->rdx, w);
            int ty = wrap(client->pos.y + row * dirs->fdy + col * dirs->rdy, h);
            map_tile_t *tile = &server->prog_cfg->world_map->tiles[ty * w + tx];
            bool dummy = true;
            *size += emit_tile(NULL, 0, tile, &dummy);
            first_tile = false;
        }
    }
}

int handle_look(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);
    dir_vec_t dirs = {0};
    get_dir_vectors(client->dir, &dirs.fdx, &dirs.fdy, &dirs.rdx, &dirs.rdy);

    size_t size = 3; // '[' + ']' + '\n'
    get_size_per_tile(server, client, &size, &dirs);

    int w = server->prog_cfg->w;
    int h = server->prog_cfg->h;
    int vision = client->lvl;  // vision depth = to lvl; 

    arena_t arena = create_mem_arena(size);
    char *buf = alloc_mem_arena(&arena, size);
    if (!buf) {
        free_mem_arena(&arena);
        return -1;
    }

    size_t pos = 0;
    buf[pos++] = '[';
    bool first_tile = true;

    for (int row = 0; row <= vision; row++) {
        for (int col = -row; col <= row; col++) {
            if (!first_tile) append_str(buf, &pos, ", ");
            int tx = wrap(client->pos.x + row * dirs.fdx + col * dirs.rdx, w);
            int ty = wrap(client->pos.y + row * dirs.fdy + col * dirs.rdy, h);
            map_tile_t *tile = &server->prog_cfg->world_map->tiles[ty * w + tx];
            bool tile_first = true;
            pos += emit_tile(buf, pos, tile, &tile_first);
            first_tile = false;
        }
    }

    buf[pos++] = ']';
    buf[pos++] = '\n';

    int res = server_send(client->fd, buf, (int)pos, "handle_look");
    free_mem_arena(&arena);
    client->cd = set_client_cd(server->prog_cfg->frequency, CD_LOOK);

    return res;
}
