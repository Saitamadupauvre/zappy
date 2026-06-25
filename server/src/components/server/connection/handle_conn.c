
#include "arena.h"
#include "server.h"
#include "commands.h"

#include <stdbool.h>
#include <stdio.h>


int handle_client_connection(server_t *server, client_t *client, char *received_input)
{
    if (client->cstate == CONN) { return 0; }
    if (client->cstate == PENDING) {
        if (handle_client_handshake(server, client, received_input) != 0) {
            fprintf(stderr, "Error: handshake with client [%d] failed.\n", client->fd);
            return -1;
        }
        return 0;
    }

    return -1;
}

static bool check_cd(client_t *client)
{
    if (client->cd > 0) { return false; }
    
    return true;
}

static int execute_action(server_t *server, client_t *client)
{
    if (client->state == CHANNELING) {
        return client->cd <= 0 ? finish_incantation(server, client) : 0;
    }

    if (!check_cd(client) || client->commands.cmd_count == 0) { return 0; }

    char *cmd = client->commands.cmd[0];
    pop_commands(client, 0);

    return handle_player_commands(server, client, cmd);
}

int handle_commands_dispatch(server_t *server)
{
    for (size_t i = 0; i < server->players.size; i++) {
        // if (execute_action(server, &server->players.vdata[i]) != 0) {
        //     return -1;
        // }
        execute_action(server, &server->players.vdata[i]);
    }

    return 0;
}
