
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_mct(server_t *server, client_t *client)
{
    char args[32];

    for (int y = 0; y < server->prog_cfg->h; y++) {
        for (int x = 0; x < server->prog_cfg->w; x++) {
            snprintf(args, sizeof(args), "%d %d", x, y);
            if (handle_bct(server, client, args) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

