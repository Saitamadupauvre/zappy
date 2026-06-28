
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>

#define FUNC "handle_incantation"
#define ELEVATION_UNDERWAY "Elevation underway\n"
#define LVL_CAP 8

// [level-1] = {nb_players, linemate, deraumere, sibur, mendiane, phiras, thystame}
static const int ELEV_REQ[7][7] = {
    {1, 1, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 0, 0, 0},
    {2, 2, 0, 1, 0, 2, 0},
    {4, 1, 1, 2, 0, 1, 0},
    {4, 1, 2, 1, 3, 0, 0},
    {6, 1, 2, 3, 0, 1, 0},
    {6, 2, 2, 2, 2, 2, 1},
};

static bool check_prereqs(server_t *server, client_t *initiator, bool channeling_only)
{
    int lvl = initiator->lvl;
    if (lvl < 1 || lvl > 7) { return 0; }
    const int *req = ELEV_REQ[lvl - 1];

    int player_count = 0;
    for (size_t i = 0; i < server->players.size; i++) {
        client_t *c = &server->players.vdata[i];
        if (c->type != PLAYER || c->cstate != CONN) {
            continue;
        }

        if (channeling_only && c->state != CHANNELING) {
            continue;
        }

        if (c->pos.x != initiator->pos.x || c->pos.y != initiator->pos.y) {
            continue;
        }

        if (c->lvl != lvl) {
            continue;
        }
        player_count++;
    }

    if (player_count < req[0]) { return false; }

    map_tile_t *tile = get_tile(server, initiator->pos.x, initiator->pos.y);
    if (tile->ressources[LINEMATE]  < req[1]) { return 0; }
    if (tile->ressources[DERAUMERE] < req[2]) { return 0; }
    if (tile->ressources[SIBUR]     < req[3]) { return 0; }
    if (tile->ressources[MENDIANE]  < req[4]) { return 0; }
    if (tile->ressources[PHIRAS]    < req[5]) { return 0; }
    if (tile->ressources[THYSTAME]  < req[6]) { return 0; }

    return true;
}

static bool can_join(client_t *c, client_t *initiator)
{
    if (c->fd == initiator->fd) {
        return true;
    }

    return c->state == ALIVE;
}

static void set_participants(server_t *server, client_t *initiator, PlayerState state, int cd)
{
    for (size_t i = 0; i < server->players.size; i++) {
        client_t *c = &server->players.vdata[i];
        if (c->type != PLAYER || c->cstate != CONN) {
            continue;
        }

        if (c->pos.x != initiator->pos.x || c->pos.y != initiator->pos.y) {
            continue;
        }

        if (c->lvl != initiator->lvl) {
            continue;
        }

        if (state == CHANNELING && !can_join(c, initiator)) {
            continue;
        }

        c->state = state;
        if (cd >= 0) {
            c->cd = cd;
        }
    }
}

static void notify_gui_plv(event_sink_t *sink, client_t *c)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "plv #%d %d\n", c->fd, c->lvl);

    if (len < 0 || len >= (int)sizeof(buf)) {
        fprintf(stderr, "[finish_incantation] Error: plv snprintf failed\n");
        return;
    }
    sink_to_guis(sink, buf, len);
}

static int send_to_participants(event_sink_t *sink, server_t *server,
    client_t *initiator, const char *msg, int len)
{
    for (size_t i = 0; i < server->players.size; i++) {
        client_t *c = &server->players.vdata[i];
        if (c->type != PLAYER || c->cstate != CONN || c->state != CHANNELING) {
            continue;
        }

        if (c->pos.x != initiator->pos.x || c->pos.y != initiator->pos.y) {
            continue;
        }

        if (c->lvl != initiator->lvl) {
            continue;
        }

        if (sink_to_fd(sink, c->fd, msg, len) != 0) {
            return -1;
        }
    }

    return 0;
}

int finish_incantation(server_t *server, client_t *initiator)
{
    event_sink_t sink = make_event_sink(server);

    if (!check_prereqs(server, initiator, true)) {
        send_to_participants(&sink, server, initiator, "ko\n", 3);
        set_participants(server, initiator, ALIVE, -1);
        return handle_pie(server, initiator, false);
    }

    int lvl = initiator->lvl;
    const int *req = ELEV_REQ[lvl - 1];
    map_tile_t *tile = get_tile(server, initiator->pos.x, initiator->pos.y);

    tile->ressources[LINEMATE]  -= req[1];
    tile->ressources[DERAUMERE] -= req[2];
    tile->ressources[SIBUR]     -= req[3];
    tile->ressources[MENDIANE]  -= req[4];
    tile->ressources[PHIRAS]    -= req[5];
    tile->ressources[THYSTAME]  -= req[6];

    char lvl_msg[32];
    int lvl_len = snprintf(lvl_msg, sizeof(lvl_msg), "Current level: %d\n", lvl + 1);
    send_to_participants(&sink, server, initiator, lvl_msg, lvl_len);

    for (size_t i = 0; i < server->players.size; i++) {
        client_t *c = &server->players.vdata[i];
        if (c->type != PLAYER || c->cstate != CONN || c->state != CHANNELING) {
            continue;
        }

        if (c->pos.x != initiator->pos.x || c->pos.y != initiator->pos.y) {
            continue;
        }

        if (c->lvl != lvl) {
            continue;
        }

        c->lvl++;
        if (c->lvl == LVL_CAP) {
            server->teams[c->team_idx].max_lvls++;
        }

        notify_gui_plv(&sink, c);
        c->state = ALIVE;
    }

    return handle_pie(server, initiator, true);
}

int handle_incantation(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);

    if (!check_prereqs(server, client, false)) {
        return sink_to_player(&sink, client, "ko\n", 3);
    }

    int cd = set_client_cd(server->prog_cfg->frequency, CD_INCANTATION);
    set_participants(server, client, CHANNELING, cd);

    if (handle_pic(server, client) != 0) { return -1; }

    return send_to_participants(&sink, server, client, ELEVATION_UNDERWAY,
        strlen(ELEVATION_UNDERWAY));
}
