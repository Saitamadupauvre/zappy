
#include "server.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>


int handle_bct(server_t *server, client_t *client, char *args)
{
    int x = 0;
    int y = 0;

    if (args == NULL || sscanf(args, "%d %d", &x, &y) != 2) {
        if (client) { return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_bct"); }
        return -1;
    }

    if (x < 0 || x >= server->prog_cfg->w ||
        y < 0 || y >= server->prog_cfg->h) {
        if (client) { return server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_bct"); }
        return -1;
    }

    map_tile_t *tile = &server->prog_cfg->world_map->tiles[y * server->prog_cfg->w + x];

    char buf[128];
    int len = snprintf(buf, sizeof(buf),
        "bct %d %d %d %d %d %d %d %d %d\n",
        x, y,
        tile->ressources[FOOD],
        tile->ressources[LINEMATE],
        tile->ressources[DERAUMERE],
        tile->ressources[SIBUR],
        tile->ressources[MENDIANE],
        tile->ressources[PHIRAS],
        tile->ressources[THYSTAME]);

    if (client == NULL)
        return group_gui_send(server, buf, len, "handle_bct");
    return server_send(client->fd, buf, len, "handle_bct");
}
