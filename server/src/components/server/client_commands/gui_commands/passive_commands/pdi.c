
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_pdi(server_t *server, int player_fd)
{
    char buffer[32];

    int len = snprintf(buffer, sizeof(buffer), "pdi #%d\n", player_fd);
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pdi] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, len, "handle_pdi") != 0) { return -1; }

    return 0;
}
