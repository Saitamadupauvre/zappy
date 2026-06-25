
#include "arena.h"
#include "config.h"
#include "server.h"
#include "commands.h"
#include "world.h"

#include <string.h>


static size_t find_poll_idx(server_t *server, int fd)
{
    for (size_t i = 0; i < server->pollds.size; i++) {
        if (server->pollds.vdata[i].fd == fd) {
            return i;
        }
    }

    return (size_t)-1;
}

static int handle_player_death(server_t *server, client_t *client, size_t *x)
{
    if (client->inv[FOOD] > 0) { return 0; }

    client->state = DEAD;
    size_t poll_idx = find_poll_idx(server, client->fd);
    if (poll_idx == (size_t)-1) { return -1; }
    
    if (server_send(client->fd, DEAD_MSG, strlen(DEAD_MSG), "handle_player_death") != 0) {
        disconnect_client(server, poll_idx);
        return -1;
    }

    if (handle_pdi(server, client->fd) != 0) {
        disconnect_client(server, poll_idx);
        return -1;
    }

    disconnect_client(server, poll_idx); 

    (*x)--;
    return 0;
}

static void handle_feeding_player(server_t *server, client_t *client, long now)
{
    if (now >= client->food_dealine) {
        client->inv[FOOD]--;
        client->food_dealine = now + (126 * 1000L / server->prog_cfg->frequency);
    }
}

static void handle_player_cd(client_t *client, long elapsed_ms)
{
    if (client->cd <= 0) { return; }

    client->cd -= (int)elapsed_ms;
    if (client->cd < 0) { client->cd = 0; }
}

int handle_player_stats(server_t *server)
{
    static long last_tick = 0;
    long now = now_ms();
    long elapsed = now - last_tick;
    last_tick = now;

    for (size_t x = 0; x < server->players.size; x++) {
        client_t *client = &server->players.vdata[x];

        handle_player_cd(client, elapsed);
        handle_feeding_player(server, client, now);
        if (handle_player_death(server, client, &x) != 0) {
            return -1;
        }
    }

    return 0;
}
