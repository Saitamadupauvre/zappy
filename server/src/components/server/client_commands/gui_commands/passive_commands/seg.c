
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_seg(server_t *server, const char *team_name)
{
    char buffer[128];
    int buffer_len = snprintf(buffer, sizeof(buffer), "seg %s\n", team_name);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_seg] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_seg") != 0) { return -1; }
    return 0;
}
