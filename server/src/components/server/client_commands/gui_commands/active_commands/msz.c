
#include "server.h"
#include "commands.h"
#include "errors.h"

#include <stdio.h>


int handle_msz(server_t *server, client_t *client)
{
    char buffer[64];

    int res = snprintf(buffer, sizeof(buffer), "msz %d %d\n",
            server->prog_cfg->w, server->prog_cfg->h);
    if (res < 0 || res > (int)sizeof(buffer)) {
        fprintf(stderr, "Error: snprintf failed in handle_msz.\n");
        return -1;
    }

    if (server_send(client->fd, buffer, res, "handle_msz") != 0) {
        fprintf(stderr, "[handle_msz] Error: %s.\n", server_errors_to_str(SERVER_SEND_FAILED));
        return -1;
    }

    return 0;
}

