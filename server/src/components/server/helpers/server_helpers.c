
#include "arena.h"
#include "errors.h"
#include "server.h"
#include "vector.h"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENT_ARENA_SIZE (1024 * 64)


int server_send(int target_fd, const char *content, int content_len, const char *where)
{
    if (send(target_fd, content, content_len, 0) < 0) {
        fprintf(stderr, "[%s] Error: %s.\n", where ,server_errors_to_str(SERVER_SEND_FAILED));
        return -1;
    }

    return 0;
}

struct pollfd init_polls(int handle_fd)
{
    struct pollfd pollfd = {
        .fd = handle_fd,
        .events = POLLIN,
    };

    return pollfd;
}

client_t init_client(int client_fd)
{
    client_t client = {0};
    client.fd = client_fd;
    client.arena = create_mem_arena(CLIENT_ARENA_SIZE);
    client.commands.cmd_count = 0;
    client.cstate = PENDING;

    return client;
}

void disconnect_client(server_t *server, size_t poll_idx)
{
    int fd = server->pollds.vdata[poll_idx].fd;
    client_t *client = find_client(server, fd);

    close(fd);
    printf("Client fd [%d] disconnected.\n", fd);

    if (client != NULL) { free_mem_arena(&client->arena); }

    vector_remove(&server->pollds, poll_idx);

    for (size_t i = 0; i < server->homeless.size; i++) {
        if (server->homeless.vdata[i].fd == fd) {
            vector_remove(&server->homeless, i);
            server->curr_connected--;
            return;
        }
    }
    for (size_t i = 0; i < server->players.size; i++) {
        if (server->players.vdata[i].fd == fd) {
            client_t *c = &server->players.vdata[i];
            if (c->team_idx >= 0) {
                server->teams[c->team_idx].player_count--;
            }
            vector_remove(&server->players, i);
            server->curr_connected--;
            return;
        }
    }
    for (size_t i = 0; i < server->guis.size; i++) {
        if (server->guis.vdata[i].fd == fd) {
            vector_remove(&server->guis, i);
            server->curr_connected--;
            return;
        }
    }
}

client_t *find_client(server_t *server, int fd)
{
    for (size_t i = 0; i < server->homeless.size; i++)
        if (server->homeless.vdata[i].fd == fd)
            return &server->homeless.vdata[i];
    for (size_t i = 0; i < server->players.size; i++)
        if (server->players.vdata[i].fd == fd)
            return &server->players.vdata[i];
    for (size_t i = 0; i < server->guis.size; i++)
        if (server->guis.vdata[i].fd == fd)
            return &server->guis.vdata[i];
    return NULL;
}
