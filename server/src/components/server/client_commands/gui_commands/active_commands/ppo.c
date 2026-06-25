
#include "server.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <time.h>


int handle_ppo(server_t *server, client_t *client, char *arg)
{
    int cfd = parse_gui_id(arg);
    client_t *target = find_client(server, cfd);
    if (target == NULL || target->type != PLAYER) {
        return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_ppo");
    }

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "ppo #%d %d %d %d\n",
        target->fd,
        target->pos.x,
        target->pos.y,
        target->dir + 1); // ENUM + 1 to match protocol

    if (len < 0 || len >= (int)sizeof(buf)) {
        fprintf(stderr, "[ppo] snprintf failed\n");
        return -1;
    }

    return server_send(client->fd, buf, len, "handle_ppo");
}

