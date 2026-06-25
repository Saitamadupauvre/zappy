
#include "config.h"
#include "server.h"
#include "commands.h"

#include <stdio.h>

int handle_sst(server_t *server, client_t *client, char *arg)
{
    char buffer[32];
    int new_freq = char_to_int(arg);
    server->prog_cfg->frequency = new_freq;

    int res = snprintf(buffer, sizeof(buffer), "sst %d\n", server->prog_cfg->frequency);
    if (res < 0 || res >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_sst] Error: snprintf failed.\n");
        return -1;
    }

    return server_send(client->fd, buffer, res, "handle_sst");
}

