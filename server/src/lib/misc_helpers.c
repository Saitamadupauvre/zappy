
#include "config.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define GUI_STR "GUI"
#define PLAYER_STR "PLAYER"
#define UNKNOWN_STR "UNKNOWN"


void server_cleanup(server_t *server)
{
    destroy_vector(&server->homeless);
    destroy_vector(&server->players);
    destroy_vector(&server->guis);
    destroy_vector(&server->eggs);
    destroy_vector(&server->pollds);
}

void server_shutdown(server_t *server)
{
    while (server->pollds.size > 1) {
        disconnect_client(server, 1);
    }

    close(server->server_fd);
    server_cleanup(server);
}

int set_client_cd(int freq, int cd)
{
    return (cd * 1000) / freq;
}

// GUI player-id arguments arrive with the protocol '#' prefix (e.g. "#5").
// char_to_int chokes on the '#'; skip it, then parse the numeric id.
// Returns -1 on malformed input so callers reply with the param error.
int parse_gui_id(char *arg)
{
    char *endptr = NULL;
    long id = 0;

    if (arg == NULL) { return -1; }
    if (*arg == '#') { arg++; }
    errno = 0;
    id = strtol(arg, &endptr, 10);
    if (*arg == '\0' || *endptr != '\0' || errno == ERANGE || id <= 0) {
        return -1;
    }
    return (int)id;
}

map_tile_t *get_tile(server_t *server, int x, int y)
{
    return &server->prog_cfg->world_map->tiles[y * server->prog_cfg->w + x];
}

char *get_args(char *command, size_t cmd_len)
{
    char *arg = command + cmd_len;

    while (*arg == ' ' || *arg == '\t') {
        arg++;
    }

    return *arg == '\0' ? NULL : arg;
}

long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void print_team_names(prog_cfg_t *prog_cfg)
{
    if (prog_cfg->team_names != NULL) {
        for (int i = 0; prog_cfg->team_names[i] != NULL; i++) {
            printf("Team[%d]: %s.\n", i, prog_cfg->team_names[i]);
        }
    }

}

int char_to_int(char *str)
{
    errno = 0;
    char *endptr = NULL;
    long integer_val = strtol(str, &endptr, 10);

    if (*str == '\0' || *endptr != '\0') {
        fprintf(stderr, "Error: char_to_int failed\n" );
        return false;
    }

    if (errno == ERANGE || integer_val <= 0) {
        fprintf(stderr, "Error: Value out of range or invalid for flag.\n");
        return false;
    }

    return integer_val;
}

