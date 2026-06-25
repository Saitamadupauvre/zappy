
#include "server.h"
#include "commands.h"
#include "event_sink.h"

#include <string.h>


int handle_left(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);

    // enum order NORTH, EAST, SOUTH, WEST: +3 == 90 degrees counterclockwise (left)
    client->dir = (client->dir + 3) % 4;

    if (sink_to_player(&sink, client, COMMAND_OK, strlen(COMMAND_OK)) != 0) {
        return -1;
    }

    return sink_player_pos(&sink, client);
}
