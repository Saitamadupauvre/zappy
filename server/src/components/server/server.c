
#include "arena.h"
#include "commands.h"
#include "config.h"
#include "server.h"
#include "errors.h"
#include "vector.h"
#include "protocol.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/poll.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static int move_homeless_client(server_t *server, client_t *client, int fd)
{
    client_t moved = *client;
    for (size_t j = 0; j < server->homeless.size; j++) {
        if (server->homeless.vdata[j].fd == fd) {
            vector_remove(&server->homeless, j);
            break;
        }
    }
    if (moved.type == GUI) {
        if (vector_pushb(&server->guis, moved) < 0) {
            fprintf(stderr, "Error: failed to add GUI client [%d]\n", fd);
            return -1;
        }
    } else {
        if (vector_pushb(&server->players, moved) < 0) {
            fprintf(stderr, "Error: failed to add player client [%d]\n", fd);
            return -1;
        }
    }
    return 0;
}

static void handle_client_reception(server_t *server, size_t *i)
{
    int fd = server->pollds.vdata[*i].fd;
    client_t *client = find_client(server, fd);
    if (client == NULL) {
        fprintf(stderr, "Error: client not found for fd [%d]\n", fd);
        return;
    }

    for (;;) {
        char *cmd = NULL;
        ssize_t res = recv_parser(client, &cmd);

        if (res == CONNECTION_CLOSED_VAL || res == RECV_FAILED_VAL) {
            disconnect_client(server, *i);
            (*i)--;
            break;
        }

        if (res == WRONG_PROTOCOL_FORMAT_VAL || cmd == NULL) {
            fprintf(stderr, "Error: format was wrong or cmd was NULL");
            break;
        }

        bool was_pending = client->cstate == PENDING;
        if (handle_client_connection(server, client, cmd) != 0) {
            disconnect_client(server, *i);
            (*i)--;
            break;
        }

        if (was_pending && client->cstate == CONN) {
            if (move_homeless_client(server, client, fd) != 0) {
                close(fd);
                vector_remove(&server->pollds, *i);
                (*i)--;
                break;
            }
            client = find_client(server, fd);
        }

        if (!was_pending && client->cstate == CONN && client->type == PLAYER) { push_commands(client, cmd); }
        if (!was_pending && client->cstate == CONN && client->type == GUI) {
            if (handle_gui_commands(server, client, cmd) != 0) {
                disconnect_client(server, *i);
                (*i)--;
                break;
            }
        }

        if (client->recv_stored == 0) { break; }
    }
}

static void handle_server_reception(server_t *server)
{
    struct sockaddr_in client_addr;
    socklen_t len_addr = sizeof(client_addr);

    int client_fd = accept(server->server_fd,
        (struct sockaddr *)&client_addr, &len_addr);
    if (client_fd < 0) {
        fprintf(stderr, "Error: accepting client connection failed\n");
        return;
    }

    if (server_send(client_fd, WELCOME_MSG, strlen(WELCOME_MSG), "handle_server_reception") < 0) {
        close(client_fd);
        return;
    }

    client_t client = init_client(client_fd);
    if (client.arena.base == NULL) {
        fprintf(stderr, "Error: initializing client failed\n");
        close(client_fd);
        return;
    }

    if (vector_pushb(&server->homeless, client) < 0) {
        fprintf(stderr, "Error: homeless vector push failed\n");
        free_mem_arena(&client.arena);
        close(client_fd);
        return;
    }

    struct pollfd pfd = init_polls(client_fd);
    if (vector_pushb(&server->pollds, pfd) < 0) {
        fprintf(stderr, "Error: pollfd vector push failed\n");
        close(client_fd);
        return;
    }

    printf("New client connected from [%s] fd [%d]\n",
        inet_ntoa(client_addr.sin_addr), client_fd);
}

static int net_poll(server_t *server)
{
    int res = poll(server->pollds.vdata, server->pollds.size, 100);

    if (res < 0) {
        if (errno == EINTR) { return 0; }
        fprintf(stderr, "Error: polling failed\n");
        return 0;
    }

    for (size_t i = 0; i < server->pollds.size; i++) {
        if (!(server->pollds.vdata[i].revents & POLLIN)) { continue; }

        if (server->pollds.vdata[i].fd == server->server_fd) {
            handle_server_reception(server);
        } else {
            handle_client_reception(server, &i);
        }
    }

    return 0;
}

int run_server(prog_cfg_t *prog_cfg, server_t *server)
{
    printf("Server listening on port [%d] with fd [%d]\n", prog_cfg->port, server->server_fd);

    if (setup_signal_handlers() != 0) {
        server_cleanup(server);
        return -1;
    }

    while (!shutdown_requested()) {
        net_poll(server);
        if (game_tick(server)) { break; }
    }

    printf("Shutting down gracefully...\n");
    server_shutdown(server);

    return 0;
}
