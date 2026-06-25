
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <string.h>
#include <stdio.h>


static int notify_guis_new_inv(server_t *server, client_t *client)
{
    char fd_buffer[12];
    int len = snprintf(fd_buffer, sizeof(fd_buffer), "%d", client->fd);
    if (len < 0 || len >=(int)sizeof(fd_buffer)) {
        fprintf(stderr, "[notify_guis_new_inv] Error: fprintf failed.\n");
        return -1;
    }

    return handle_pin(server, client, fd_buffer);
}

int handle_set(server_t *server, client_t *client, char *args)
{
    event_sink_t sink = make_event_sink(server);
    int res = str_to_ressource(args);
    if (res < 0)
        return sink_to_player(&sink, client, "ko\n", 3);

    if (client->inv[res] <= 0) {
        client->cd = set_client_cd(server->prog_cfg->frequency, CD_SET);
        return sink_to_player(&sink, client, "ko\n", 3);
    }

    int w = server->prog_cfg->w;
    map_tile_t *tile = &server->prog_cfg->world_map->tiles[
        client->pos.y * w + client->pos.x];

    client->inv[res]--;
    tile->ressources[res]++;
    client->cd = set_client_cd(server->prog_cfg->frequency, CD_SET);

    if (handle_pdr(server, client, (Ressources)res)) { return -1; }
    if (notify_guis_new_inv(server, client) != 0) { return -1; }

    return sink_to_player(&sink, client, COMMAND_OK, strlen(COMMAND_OK));
}
