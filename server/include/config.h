
#ifndef CONFIG_H
    #define CONFIG_H

    #define EXIT_SUCCESS 0
    #define EXIT_FAILED 84

    #include <stdint.h>
    #include <stdbool.h>

    #define PORT_MIN 1
    #define PORT_MAX 65535

    #define HELP_FLAG "--help"
    #define USAGE "USAGE: ./zappy_server -p port -x width -y height -n"\
"name1 name2 ... -c clientsNb -f freq"

typedef struct prog_cfg_s {
    uint16_t port;
    int w;
    int h;
    int client_count;
    int frequency;
    char **team_names;
    int team_count;
} prog_cfg_t;


int handle_pre_serv_proc(int argc, char *argv[]);

bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg);


#endif
