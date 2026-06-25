
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>

#define FUNC "handle_eject"

static int dir_to_k(Direction dir)
{
    switch (dir) {
        case NORTH: return 3;
        case EAST:  return 4;
        case SOUTH: return 1;
        case WEST:  return 2;
    }

    return 1;
}

static void push_to(server_t *server, client_t *victim, Direction dir)
{
    int dx = 0, dy = 0;
    int w = server->prog_cfg->w;
    int h = server->prog_cfg->h;
    map_tile_t *tiles = server->prog_cfg->world_map->tiles;

    switch (dir) {
        case NORTH: dx =  0; dy = -1; break;
        case SOUTH: dx =  0; dy =  1; break;
        case EAST:  dx =  1; dy =  0; break;
        case WEST:  dx = -1; dy =  0; break;
    }

    tiles[victim->pos.y * w + victim->pos.x].player_count--;
    victim->pos.x = ((victim->pos.x + dx) % w + w) % w;
    victim->pos.y = ((victim->pos.y + dy) % h + h) % h;
    tiles[victim->pos.y * w + victim->pos.x].player_count++;

    if (victim->state == CHANNELING) { victim->state = ALIVE; }
}

static int eject_players(event_sink_t *sink, server_t *server,
    client_t *client, int k)
{
    int ejected = 0;
    char eject_msg[16];
    int eject_len = snprintf(eject_msg, sizeof(eject_msg), "eject: %d\n", k);

    for (size_t i = 0; i < server->players.size; i++) {
        client_t *victim = &server->players.vdata[i];
        if (victim->fd == client->fd) { continue; }
        if (victim->type != PLAYER || victim->cstate != CONN) { continue; }
        if (victim->pos.x != client->pos.x || victim->pos.y != client->pos.y) { continue; }

        push_to(server, victim, client->dir);
        if (sink_to_fd(sink, victim->fd, eject_msg, eject_len) != 0) { return -1; }

        char pex_buf[32];
        int pex_len = snprintf(pex_buf, sizeof(pex_buf), "pex #%d\n", victim->fd);
        if (sink_to_guis(sink, pex_buf, pex_len) != 0) { return -1; }
        if (sink_player_pos(sink, victim) != 0) { return -1; }
        ejected++;
    }

    return ejected;
}

static int destroy_eggs(server_t *server, client_t *client)
{
    int destroyed = 0;

    for (size_t i = 0; i < server->eggs.size; i++) {
        egg_t *egg = &server->eggs.vdata[i];
        if (egg->x != client->pos.x || egg->y != client->pos.y) { continue; }
        int egg_id = egg->egg_id;
        int team_idx = egg->team_idx;
        vector_remove(&server->eggs, i);
        i--;
        get_tile(server, client->pos.x, client->pos.y)->egg_count--;
        server->teams[team_idx].egg_count--;
        server->teams[team_idx].team_slots--;
        if (handle_edi(server, egg_id) != 0) { return -1; }
        destroyed++;
    }

    return destroyed;
}

int handle_eject(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);
    int k = dir_to_k(client->dir);

    int ep = eject_players(&sink, server, client, k);
    if (ep < 0) { return -1; }

    int ed = destroy_eggs(server, client);
    if (ed < 0) { return -1; }

    const char *resp = (ep + ed) > 0 ? COMMAND_OK : "ko\n";
    client->cd = set_client_cd(server->prog_cfg->frequency, CD_EJECT);

    return sink_to_player(&sink, client, resp, strlen(resp));
}
