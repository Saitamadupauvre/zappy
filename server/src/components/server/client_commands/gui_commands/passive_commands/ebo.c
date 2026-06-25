
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_ebo(server_t *server, int egg_id)
{
    char buffer[32];
    int buffer_len = snprintf(buffer, sizeof(buffer), "ebo #%d\n", egg_id);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_ebo] Error: snprintf failed.\n");
        return -1;
    }

    if (group_gui_send(server, buffer, buffer_len, "handle_ebo") != 0) { return -1; }
    return 0;
}
