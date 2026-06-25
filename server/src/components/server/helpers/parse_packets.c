
#include "arena.h"
#include "server.h"
#include "errors.h"
#include "vector.h"
#include "protocol.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


ssize_t my_recv(int fd, char *buffer, size_t buffer_len)
{
    ssize_t res;

    do {
        res = read(fd, buffer, buffer_len);
    } while (res < 0 && errno == EINTR);

    if (res > 0) { return res; }
    if (res == 0) { return CONNECTION_CLOSED_VAL; }

    fprintf(stderr, "Error: %s.\n", packet_errors_to_str(RECV_FAILED));

    return RECV_FAILED_VAL;
}

static int flush_current_buffer(client_t *client, char **command)
{
    char *end_format = strstr(client->recept_buffer, "\r\n"); //telnet
    size_t skip = 2;
    if (!end_format) {
        end_format = strstr(client->recept_buffer, PROTOCOL_DELIM);
        skip = 1;
    }

    if (!end_format) { return WRONG_PROTOCOL_FORMAT_VAL; }

    size_t cmd_size = end_format - client->recept_buffer;
    *command = alloc_mem_arena(&client->arena, cmd_size + 1);
    if (*command == NULL) { 
        fprintf(stderr, "Error: %s on for [%s] command.\n", alloc_errors_to_str(COMMAND_ALLOC_FAILED), *command);
        return RECV_FAILED_VAL;
    }

    memcpy(*command, client->recept_buffer, cmd_size);
    (*command)[cmd_size] = '\0';

    size_t consumed = cmd_size + skip;
    memmove(client->recept_buffer, client->recept_buffer + consumed,
            client->recv_stored - consumed);
    client->recv_stored -= consumed;
    client->recept_buffer[client->recv_stored] = '\0';
    return (ssize_t)cmd_size;
}

ssize_t recv_parser(client_t *client, char **command)
{
    ssize_t res = flush_current_buffer(client, command);
    if (res != WRONG_PROTOCOL_FORMAT_VAL) { return res; }

    res = my_recv(
            client->fd,
            client->recept_buffer + client->recv_stored,
            sizeof(client->recept_buffer) - client->recv_stored - 1
    );
    if (res <= 0) { return res; }

    client->recv_stored += res;
    client->recept_buffer[client->recv_stored] = '\0';

    return flush_current_buffer(client, command);
}
