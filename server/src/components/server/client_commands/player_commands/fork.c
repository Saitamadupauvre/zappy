
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>


static egg_t laying_egg(server_t *server, client_t *client)
{
    egg_t egg = {
        .egg_id    = server->next_egg_id++,
        .player_fd = client->fd,
        .team_idx  = client->team_idx,
        .x         = client->pos.x,
        .y         = client->pos.y,
    };

    return egg;
}

int handle_fork(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);

    if (handle_pfk(server, client) != 0) { return -1; }

    egg_t egg = laying_egg(server, client);
    if (vector_pushb(&server->eggs, egg) != 0) {
        fprintf(stderr, "[handle_fork] Error: egg vector full.\n");
        return -1;
    }

    map_tile_t *tile = get_tile(server, client->pos.x, client->pos.y);
    tile->egg_count++;
    server->teams[client->team_idx].egg_count++;
    server->teams[client->team_idx].team_slots++;

    client->cd = set_client_cd(server->prog_cfg->frequency, CD_FORK);

    if (handle_enw(server, client, egg.egg_id) != 0) { return -1; }

    return sink_to_player(&sink, client, COMMAND_OK, strlen(COMMAND_OK));
}
