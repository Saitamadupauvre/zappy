
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_pfk(server_t *server, client_t *client)
{
    char buffer[32];

    int buffer_len = snprintf(buffer, sizeof(buffer), "pfk #%d\n", client->fd);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pfk] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_pfk") != 0) { return -1; }

    return 0;
}
