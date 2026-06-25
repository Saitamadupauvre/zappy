
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_pnw(server_t *server, client_t *client)
{
    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "pnw #%d %d %d %d %d %s\n",
        client->fd,
        client->pos.x,
        client->pos.y,
        client->dir + 1,
        client->lvl,
        client->team_name);
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pnw] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, len, "handle_pnw") != 0) { return -1; }

    return 0;
}
