
#include "server.h"

#include <stdio.h>


int handle_stu(server_t *server)
{
    char buffer[24];
    int len = snprintf(buffer, sizeof(buffer), "%lu\n", server->uptime);
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_stu] Error: snprintf failed.\n");
        return -1;
    }

    return group_gui_send(server, buffer, len, "handle_stu");
}
