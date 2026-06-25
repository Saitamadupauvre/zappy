
#include "commands.h"
#include "server.h"

#include <stdbool.h>
#include <unistd.h>

#define MIN_PLAYERS_MAX 6

static void mass_disconnect(server_t *server)
{
    for (; server->pollds.size > 0;) {
        disconnect_client(server, 0);
    }

    close(server->server_fd);
}

static int check_win_condition(server_t *server)
{
    for (int i = 0; i < server->prog_cfg->team_count; i++) {
        if (server->teams[i].max_lvls >= MIN_PLAYERS_MAX) {
            server->game_over = true;
            return i;
        }
    }

    return -1;
}

bool handle_win_con(server_t *server)
{
    int res = check_win_condition(server);
    if (res == -1) { return false; }

    if (handle_seg(server, server->prog_cfg->team_names[res]) != 0) {
        return false;
    }

    mass_disconnect(server);

    return true;
}
