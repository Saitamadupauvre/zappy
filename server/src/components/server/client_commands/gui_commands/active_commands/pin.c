
#include "server.h"
#include "commands.h"
#include "world.h"

#include <stdio.h>
#include <string.h>


int handle_pin(server_t *server, client_t *client, char *arg)
{
    int cfd = parse_gui_id(arg);
    client_t *target = find_client(server, cfd);
    if (target == NULL || target->type != PLAYER) {
        return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_pin");
    }

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "pin #%d %d %d %d %d %d %d %d %d %d\n",
        target->fd,
        target->pos.x,
        target->pos.y,
        target->inv[FOOD],
        target->inv[LINEMATE],
        target->inv[DERAUMERE],
        target->inv[SIBUR],
        target->inv[MENDIANE],
        target->inv[PHIRAS],
        target->inv[THYSTAME]); 
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[pin] snprintf failed\n");
        return -1;
    }

    return group_gui_send(server, buffer, len, "handle_pin");
}

