
#ifndef CONFIG_H
    #define CONFIG_H

    #include <stdint.h>
    #include <stdbool.h>

    #include "arena.h"

    #define EXIT_SUCCESS 0
    #define EXIT_FAILED 84

    #define PORT_MIN 1
    #define PORT_MAX 65535
    #define DEFAULT_FREQUENCY 100

    #define HELP_FLAG "--help"
    #define USAGE "USAGE: ./zappy_server -p port -x width -y height -n"\
                    "name1 name2 ... -c clientsNb -f freq"
    #define PERM_ARENA_SIZE (1024 * 1024)   // 1MB should be ok

typedef struct world_map_s world_map_t;

typedef struct prog_cfg_s {
    arena_t perm_mem_arena;
    uint16_t port;
    int w;
    int h;
    int client_count;
    int frequency;
    char **team_names;
    int team_count;
    world_map_t *world_map;
} prog_cfg_t;

int handle_pre_serv_proc(int argc, char *argv[]);

bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg);

//HELPERS

void print_team_names(prog_cfg_t *prog_cfg);

int char_to_int(char *str);

long now_ms(void);

#endif
