
#include "server.h"
#include "config.h"
#include "errors.h"
#include "protocol.h"

#include <stdio.h>
#include <string.h>

#define FUNC "handle_client_handshake"

int reject_client(client_t *client, const char *reason)
{
    printf("%s client [%d] rejected.\n", reason, client->fd);
    if (server_send(client->fd, KO, strlen(KO), FUNC) != 0) { return print_server_errors(SERVER_SEND_FAILED, FUNC); }
    client->cstate = REJECTED;

    return -1;
}

int check_team_slot(server_t *server, int team_idx)
{
    team_info_t *team = &server->teams[team_idx];

    return team->team_slots - team->player_count + team->egg_count;
}

int find_team(server_t *server, const char *team)
{
    for (int i = 0; i < server->prog_cfg->team_count; i++) {
        if (strcmp(team, server->teams[i].team_name) == 0) {
            return i;
        }
    }

    return -1;
}
