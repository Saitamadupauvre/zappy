
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <string.h>

static void get_move_vector(Direction dir, int *dx, int *dy)
{
    switch (dir) {
        case NORTH: *dx =  0; *dy = -1; break;
        case SOUTH: *dx =  0; *dy =  1; break;
        case EAST:  *dx =  1; *dy =  0; break;
        case WEST:  *dx = -1; *dy =  0; break;
    }
}

static int wrap(int v, int max)
{
    return ((v % max) + max) % max;
}

static void move_player(server_t *server, client_t *client)
{
    int dx = 0;
    int dy = 0;
    int w = server->prog_cfg->w;
    int h = server->prog_cfg->h;
    map_tile_t *tiles = server->prog_cfg->world_map->tiles;

    get_move_vector(client->dir, &dx, &dy);
    tiles[client->pos.y * w + client->pos.x].player_count--;
    client->pos.x = wrap(client->pos.x + dx, w);
    client->pos.y = wrap(client->pos.y + dy, h);
    tiles[client->pos.y * w + client->pos.x].player_count++;
}

int handle_forward(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);

    move_player(server, client);
    client->cd = set_client_cd(server->prog_cfg->frequency, CD_FORWARD);

    if (sink_to_player(&sink, client, COMMAND_OK, strlen(COMMAND_OK)) != 0) {
        return -1;
    }

    return sink_player_pos(&sink, client);
}
