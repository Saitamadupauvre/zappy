
#include "server.h"
#include "commands.h"
#include "errors.h"

#include <stdio.h>

int handle_tna(server_t *server, client_t *client)
{
    int res = 0;

    for (int x = 0; x < server->prog_cfg->team_count; x++) {
        char buffer[128];

        res = snprintf(buffer, sizeof(buffer), "tna %s\n", server->prog_cfg->team_names[x]);
        if (res < 0 || res >= (int)sizeof(buffer)) {
            fprintf(stderr, "[handle_tna] Error: snprintf failed.\n");
            return -1;
        }

        if (server_send(client->fd, buffer, res, "handle_tna") != 0) {
            fprintf(stderr, "[handle_tna] Error: %s.\n", server_errors_to_str(SERVER_SEND_FAILED));
            return -1;
        }
    }

    return 0;
}

