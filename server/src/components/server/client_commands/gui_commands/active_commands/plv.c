
#include "server.h"
#include "commands.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <time.h>


int handle_plv(server_t *server, client_t *client, char *arg)
{
    int cfd = parse_gui_id(arg);
    client_t *target = find_client(server, cfd);
    if (target == NULL || target->type != PLAYER) {
        return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_plv");
    }

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "plv #%d %d\n", target->fd, target->lvl); 
    if (len < 0 || len >= (int)sizeof(buf)) {
        fprintf(stderr, "[plv] snprintf failed\n");
        return -1;
    }

    return server_send(client->fd, buf, len, "handle_plv");
}
