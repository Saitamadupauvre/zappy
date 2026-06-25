
#include "server.h"
#include "commands.h"
#include "event_sink.h"

#include <stdio.h>

#define FUNC "handle_connect_nbr"


int handle_connect_nbr(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);
    int slots = check_team_slot(server, client->team_idx);
    if (slots < 0) { slots = 0; }

    char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", slots);
    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[handle_connect_nbr] Error: snprintf failed.\n");
        return -1;
    }

    client->cd = 0;

    return sink_to_player(&sink, client, buffer, len);
}
