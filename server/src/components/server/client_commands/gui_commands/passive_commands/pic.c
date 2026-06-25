
#include "server.h"
#include "commands.h"

#include <stdio.h>


int handle_pic(server_t *server, client_t *client)
{
    char buffer[256];

    int buffer_len = snprintf(buffer, sizeof(buffer), "pic %d %d %d",
        client->pos.x, client->pos.y, client->lvl);
    if (buffer_len < 0 || buffer_len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_pic] Error: snprintf failed.\n");
        return -1;
    }

    for (size_t x = 0; x < server->players.size; x++) {
        client_t *pclient = &server->players.vdata[x];
        if (pclient->type != PLAYER || pclient->cstate != CONN) { continue; }
        if (pclient->pos.x != client->pos.x || pclient->pos.y != client->pos.y) { continue; }
        if (pclient->lvl != client->lvl) { continue; }

        int buf_write = snprintf(buffer + buffer_len, sizeof(buffer) - buffer_len, " #%d", pclient->fd);
        if (buf_write < 0 || buf_write >= (int)(sizeof(buffer) - buffer_len)) {
            fprintf(stderr, "[handle_pic] Error: snprintf failed\n");
            return -1;
        }

        buffer_len += buf_write;
    }

    if (buffer_len >= (int)sizeof(buffer) - 1) {
        fprintf(stderr, "[handle_pic] Error: buffer full.\n");
        return -1;
    }

    buffer[buffer_len++] = '\n';
    buffer[buffer_len] = '\0';

    if (group_gui_send(server, buffer, buffer_len, "handle_pic") != 0) { return -1; }

    return 0;
}
