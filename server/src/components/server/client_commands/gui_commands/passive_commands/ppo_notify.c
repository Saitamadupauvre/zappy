
#include "server.h"
#include "commands.h"

#include <stdio.h>


int notify_gui_ppo(server_t *server, client_t *client)
{
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
        client->fd,
        client->pos.x,
        client->pos.y,
        client->dir + 1); // ENUM + 1 to match protocol
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[notify_gui_ppo] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, len, "notify_gui_ppo") != 0) { return -1; }

    return 0;
}
