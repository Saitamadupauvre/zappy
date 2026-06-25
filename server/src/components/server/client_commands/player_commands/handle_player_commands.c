
#include "server.h"
#include "commands.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>

static const player_command_info_t player_command_table[] = {
    {"Forward", FORWARD, false, handle_forward, NULL},
    {"Right", RIGHT, false, handle_right, NULL},
    {"Left", LEFT, false, handle_left, NULL},
    {"Look", LOOK, false, handle_look, NULL},
    {"Inventory", INVENTORY, false, handle_inventory, NULL},
    {"Broadcast", BROADCAST, true,  NULL, handle_broadcast},
    {"Connect_nbr", CONNECT_NBR, false, handle_connect_nbr, NULL},
    {"Fork", FORK, false,  handle_fork, NULL},
    {"Eject", EJECT, false, handle_eject, NULL},
    {"Take", TAKE_OBJ, true, NULL, handle_take},
    {"Set", SET_OBJ, true, NULL, handle_set},
    {"Incantation", INCANTATION, false, handle_incantation, NULL},
    {NULL,  COMMAND_UNKNOWN, false, NULL, NULL      }
};

int handle_player_commands(server_t *server, client_t *client, char *command)
{
    event_sink_t sink = make_event_sink(server);

    for (size_t i = 0; player_command_table[i].cmd_type != COMMAND_UNKNOWN; i++) {
        const player_command_info_t *tentry = &player_command_table[i];
        size_t cmd_len = strlen(tentry->cmd_name);

        if (strncmp(command, tentry->cmd_name, cmd_len) != 0) { continue; }

        char *args = tentry->has_args ? get_args(command, cmd_len) : NULL;

        if (tentry->has_args && args == NULL) {
            return sink_to_player(&sink, client, COMMAND_PARAM, strlen(COMMAND_PARAM));
        }

        if (tentry->has_args) { return tentry->func_args(server, client, args); }
        return tentry->func(server, client);
    }

    return sink_to_player(&sink, client, UNKNOWN_COMMAND, strlen(UNKNOWN_COMMAND));
}

