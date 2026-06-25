
#include "server.h"
#include "commands.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const gui_command_info_t gui_command_table[] = {
    {"msz", MSZ, false, handle_msz, NULL, NULL},
    {"mct", MCT, false, handle_mct, NULL, NULL},
    {"tna", TNA, false, handle_tna, NULL, NULL},
    {"sgt", SGT, false, handle_sgt, NULL, NULL},
    {"bct", BCT, true,  NULL, handle_bct, NULL},
    {"sst", SST, true,  NULL, handle_sst, NULL},
    {"ppo", PPO, true,  NULL, handle_ppo, NULL},
    {"plv", PLV, true,  NULL, handle_plv, NULL},
    {"pin", PIN, true,  NULL, handle_pin, NULL},
    {"stu", STU, false, NULL, NULL, handle_stu},
    {NULL,  CMD_UNKNOWN, false, NULL, NULL, NULL}
};

int handle_gui_commands(server_t *server, client_t *client, char *command)
{
    for (size_t i = 0; gui_command_table[i].cmd_type != CMD_UNKNOWN; i++) {
        const gui_command_info_t *tentry = &gui_command_table[i];
        size_t cmd_len = strlen(tentry->cmd_name);

        if (strncmp(command, tentry->cmd_name, cmd_len) != 0) { continue; }

        char *args = tentry->has_args ? get_args(command, cmd_len) : NULL;

        if (tentry->has_args && args == NULL) {
            if (server_send(client->fd, COMMAND_PARAM, strlen(COMMAND_PARAM), "handle_gui_commands") != 0) { return -1; }
            return 0;
        }

        if (tentry->has_args) { return tentry->func_args(server, client, args); }
        if (tentry->func_custom) { return tentry->func_custom(server); }
        return tentry->func(server, client);
    }

    if (server_send(client->fd, UNKNOWN_COMMAND, strlen(UNKNOWN_COMMAND), "handle_gui_commands") != 0) { return -1; }

    return 0;
}

