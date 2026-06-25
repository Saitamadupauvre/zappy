
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_pie(server_t *server, client_t *client, bool inc_res)
{
    char buffer[64];
    int buffer_len = snprintf(buffer, sizeof(buffer), "pie %d %d %d\n",
        client->pos.x, client->pos.y, inc_res);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pie] Error: snprintf failed\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_pie") != 0) { return -1; }

    return 0;
}
