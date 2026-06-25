
#include "server.h"
#include "config.h"
#include "errors.h"
#include "commands.h"
#include "world.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define GUI_TYPE "GRAPHIC"
#define GUI_TEAM "GRAPHIC\n"

#define FUNC "handle_client_handshake"

#define REASON_NO_TEAM "team is unknown"
#define REASON_NO_SLOT "team doesnt have a free slot"


static int init_gui_client(server_t *server, client_t *client)
{
    client->type = GUI;
    client->cstate = CONN;

    if (handle_msz(server, client) != 0) { return -1; }
    if (handle_sgt(server, client) != 0) { return -1; }
    if (handle_mct(server, client) != 0) { return -1; }
    if (handle_tna(server, client) != 0) { return -1; }

    for (size_t i = 0; i < server->players.size; i++) {
        client_t *player = &server->players.vdata[i];

        char fd_str[16];
        snprintf(fd_str, sizeof(fd_str), "%d", player->fd);

        char pnw_buf[128];
        int pnw_len = snprintf(pnw_buf, sizeof(pnw_buf), "pnw #%d %d %d %d %d %s\n",
            player->fd, player->pos.x, player->pos.y,
            player->dir + 1, player->lvl, player->team_name);
        if (pnw_len < 0 || pnw_len >= (int)sizeof(pnw_buf)) { return -1; }
        if (server_send(client->fd, pnw_buf, pnw_len, "init_gui_client") != 0) { return -1; }

        if (handle_plv(server, client, fd_str) != 0) { return -1; }
        if (handle_pin(server, client, fd_str) != 0) { return -1; }
    }

    printf("Client [%d] has connected to the server as GUI !\n", client->fd);

    return 0;
}

static void spawn_player(server_t *server, client_t *client)
{
    int x = rand() % server->prog_cfg->w;
    int y = rand() % server->prog_cfg->h;
    int facing_dir = rand() % 4;

    client->pos.x = x;
    client->pos.y = y;
    client->dir = facing_dir;

    map_tile_t *tile = &server->prog_cfg->world_map->tiles[y * server->prog_cfg->w + x];
    tile->player_count++;
    printf("Player [%d] spawned on tile [%d %d].\n", client->fd, client->pos.x, client->pos.y);
}

static int init_egg_client(server_t *server, client_t *client, int team_idx)
{
    team_info_t *team = &server->teams[team_idx];

    if (team->egg_count == 0) { return 0; }

    for (size_t i = 0; i < server->eggs.size; i++) {
        if (server->eggs.vdata[i].team_idx == team_idx) {
            int egg_id = server->eggs.vdata[i].egg_id;
            int ex = server->eggs.vdata[i].x;
            int ey = server->eggs.vdata[i].y;
            vector_remove(&server->eggs, i);
            map_tile_t *tile = get_tile(server, ex, ey);
            tile->egg_count--;
            team->egg_count--;
            client->pos.x = ex;
            client->pos.y = ey;
            if (handle_ebo(server, egg_id) != 0) { return -1; }
            return 1;
        }
    }

    return 0;
}

static int init_player_client(server_t *server, client_t *client, int team_idx)
{
    int hatched = init_egg_client(server, client, team_idx);
    if (hatched < 0) { return -1; }

    const char *team = server->prog_cfg->team_names[team_idx];
    int team_name_len = strlen(team);

    client->team_name = alloc_mem_arena(&client->arena, team_name_len + 1);
    if (client->team_name == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(TEAM_NAME_ALLOC_FAILED));
        return -1;
    }
    memcpy(client->team_name, server->prog_cfg->team_names[team_idx], team_name_len);
    client->team_name[team_name_len] = '\0';

    client->cstate = CONN;
    client->type = PLAYER;
    client->state = ALIVE;
    client->lvl = 1;
    // client->inv[FOOD] = 1000;
    client->inv[FOOD] = 10;
    client->team_idx = team_idx;
    client->food_dealine = now_ms() + (126 * 1000L / server->prog_cfg->frequency);

    if (hatched) {
        client->dir = rand() % 4;
        get_tile(server, client->pos.x, client->pos.y)->player_count++;
        printf("Player [%d] hatched on tile [%d %d].\n", client->fd, client->pos.x, client->pos.y);
    } else {
        spawn_player(server, client);
    }

    server->teams[team_idx].player_count++;
    server->client_count++;
    server->curr_connected++;

    if (handle_pnw(server, client) != 0) { return -1; }
    printf("Client [%d] from team [%s] has connected to the server !\n",
        client->fd, client->team_name);

    return 0;
}

int handle_client_handshake(server_t *server, client_t *client, char *team)
{
    if (strcmp(GUI_TYPE, team) == 0) {
        if (init_gui_client(server, client) != 0) { return -1; }
        return 0;
    }

    int found_team_idx = find_team(server, team);
    if (found_team_idx < 0) { return reject_client(client, REASON_NO_TEAM);}

    int free_slot = check_team_slot(server, found_team_idx);
    if (free_slot <= 0) { return reject_client(client, REASON_NO_TEAM); }

    if (init_player_client(server, client, found_team_idx)) { return -1; }

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%d\n%d %d\n",
        check_team_slot(server, found_team_idx),
        server->prog_cfg->w,
        server->prog_cfg->h);

    if (server_send(client->fd, buf, len, FUNC) != 0) {
        return print_server_errors(SERVER_SEND_FAILED, FUNC);
    }

    return 0;
}

