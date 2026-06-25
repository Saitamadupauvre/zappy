
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_sgt(server_t *server, client_t *client)
{
    char buffer[32];

    int res = snprintf(buffer, sizeof(buffer), "sgt %d\n", server->prog_cfg->frequency);
    if (res < 0 || res >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_sgt] Error: snprintf failed.\n");
        return -1;
    }

    return server_send(client->fd, buffer, res, "handle_sgt");
}

