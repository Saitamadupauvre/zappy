
#include "arena.h"
#include "errors.h"
#include "server.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>


int handle_pbc(server_t *server, client_t *client, const char *message)
{
    if (message == NULL) { return -1; }

    int needed = snprintf(NULL, 0, "pbc #%d %s\n", client->fd, message);
    if (needed < 0) { return -1; }

    char buffer[needed + 1];
    int fmt_len = snprintf(buffer, needed + 1, "pbc #%d %s\n", client->fd, message);
    if (fmt_len < 0 || fmt_len >= needed + 1) {
        fprintf(stderr, "[handle_pbc] Error: snprintf failed\n");
        return -1;
    }

    return group_gui_send(server, buffer, fmt_len, "handle_pbc");
}

