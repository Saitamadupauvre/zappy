
#include "server.h"
#include "commands.h"

#include <stdio.h>

void pop_commands(client_t *client, int idx)
{
    for (int i = idx; i < client->commands.cmd_count - 1; i++)
        client->commands.cmd[i] = client->commands.cmd[i + 1];
    client->commands.cmd[client->commands.cmd_count - 1] = NULL;
    client->commands.cmd_count -= 1;
}

int push_commands(client_t *client, char *command)
{
    if (command == NULL) { return -1; }
    if (client->commands.cmd_count >= MAX_COMMANDS_COUNT) {
        printf("Client [%d] already has 10 commands in the buffer.\n", client->fd);
        return 0;
    }

    client->commands.cmd[client->commands.cmd_count++] = command;

    printf("Command [%s] queued for client [%d] (%d/%d)\n",
        command, client->fd, client->commands.cmd_count, MAX_COMMANDS_COUNT);

    return 0;
}
