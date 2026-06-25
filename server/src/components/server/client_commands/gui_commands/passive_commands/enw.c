
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_enw(server_t *server, client_t *client, int egg_id)
{
    char buffer[64];
    int buffer_len = snprintf(buffer, sizeof(buffer), "enw #%d #%d %d %d\n",
        egg_id, client->fd, client->pos.x, client->pos.y);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_enw] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_enw") != 0) { return -1; }
    return 0;
}
