
#include "commands.h"
#include "server.h"

#include <stdio.h>
#include <string.h>


int handle_pex(client_t *client, int victim_fd)
{
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "pex #%d\n", victim_fd);
    if (len < 0 || len >= (int)sizeof(buffer)) {
        return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_pex");
    }

    return server_send(client->fd, buffer, len, "handle_pex");
}
