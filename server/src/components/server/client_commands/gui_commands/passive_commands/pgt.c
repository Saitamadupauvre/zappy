
#include "server.h"
#include "commands.h"
#include "world.h"

#include <stdio.h>


int handle_pgt(server_t *server, client_t *client, Ressources type)
{
    char buffer[32];
    int buffer_len = snprintf(buffer, sizeof(buffer), "pgt #%d %s\n",
        client->fd, ressources_to_str(type));
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pgt] Error : fprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_pgt") != 0) { return -1; }

    return 0;
}
