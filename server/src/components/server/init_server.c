#include "arena.h"
#include "config.h"
#include "errors.h"
#include "server.h"
#include "vector.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>


static void init_teams(server_t *server, prog_cfg_t *prog_cfg)
{
    memset(server->teams, 0, sizeof(team_info_t) * prog_cfg->team_count);
    for (int i = 0; i < prog_cfg->team_count; i++) {
        server->teams[i].team_name = prog_cfg->team_names[i];
        server->teams[i].team_slots = prog_cfg->client_count;
    }
}

static int init_pollfs(server_t *server)
{
    init_vector(&server->pollds, 16);
    if (server->pollds.vdata == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return -1;
    }

    struct pollfd listener_fd = {
        .fd = server->server_fd,
        .events = POLLIN,
    };

    vector_pushb(&server->pollds, listener_fd);
    return 0;
}


int setup_server_socket(uint16_t port)
{
    int server_fd = -1;
    int opt = 1;
    struct sockaddr_in address = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Error: %s.\n", server_errors_to_str(SOCKET_CREATION_FAILED));
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        fprintf(stderr, "Error: %s.\n", server_errors_to_str(SOCKET_OPTIONS_FAILED));
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        fprintf(stderr, "Error: %s.\n", server_errors_to_str(SOCKET_BIND_FAILED));
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        fprintf(stderr, "Error: %s.\n", server_errors_to_str(SOCKET_LISTEN_FAILED));
        close(server_fd);
        return -1;
    }

    return server_fd;
}

server_t *init_server(prog_cfg_t *prog_cfg)
{
    server_t *server = alloc_mem_arena(&prog_cfg->perm_mem_arena, sizeof(server_t));
    if (server == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }
    memset(server, 0, sizeof(server_t));
    server->prog_cfg = prog_cfg;

    int max_players = prog_cfg->team_count * prog_cfg->client_count;

    server->server_fd = setup_server_socket(prog_cfg->port);
    if (server->server_fd < 0) { return NULL; }
    if (init_pollfs(server) != 0) { return NULL; }

    init_vector(&server->homeless, 8);
    if (server->homeless.vdata == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }

    init_vector(&server->players, max_players);
    if (server->players.vdata == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }

    init_vector(&server->guis, 4);
    if (server->guis.vdata == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }

    init_vector(&server->eggs, max_players);
    if (server->eggs.vdata == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }
    server->next_egg_id = 0;

    server->teams = alloc_mem_arena(&prog_cfg->perm_mem_arena, sizeof(team_info_t) * prog_cfg->team_count);
    if (server->teams == NULL) {
        fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return NULL;
    }
    init_teams(server, prog_cfg);

    printf("Server initialized on port %d with capacity for %d players.\n", prog_cfg->port, max_players);

    return server;
}
