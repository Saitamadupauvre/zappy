
#include "config.h"
#include "errors.h"
#include "parsing.h"

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>

typedef struct args_list_s {
    bool has_port;
    bool has_width;
    bool has_height;
    bool has_clients;
    bool has_teams;
    bool has_frequency;
} args_list_t;

bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg)
{
    args_list_t args = {0};
    prog_cfg->frequency = 100;
    int opt = -1;
    opterr = 0;

    while ((opt = getopt(argc, argv, "p:x:y:c:f:n")) != -1) {
        switch (opt) {
            case 'p': if (!parse_port(optarg, prog_cfg, &args.has_port)) {return false; }
                break;
            case 'x': if (!parse_ints(optarg, &prog_cfg->w, &args.has_width, INVALID_WIDTH_ARG)) { return false; }; 
                break;
            case 'y': if (!parse_ints(optarg, &prog_cfg->h, &args.has_height, INVALID_HEIGHT_ARG)) { return false; } 
                break;
            case 'c': if (!parse_ints(optarg, &prog_cfg->client_count, &args.has_clients, INVALID_NUMBER_OF_CLIENTS)) { return false; }
                break;
            case 'f': if (!parse_ints(optarg, &prog_cfg->frequency, &args.has_frequency,  INVALID_FREQUENCY)) { return false; }
                break;
            case 'n': if (!parse_teams(argc, argv, prog_cfg, &args.has_teams)) { return false; }
                break;
            case '?': return parse_invalid_flag(optopt);
            default: return false;
        }
    }

    if (prog_cfg->team_names != NULL) {
        for (int i = 0; prog_cfg->team_names[i] != NULL; i++) {
            printf("Team[%d]: %s.\n", i, prog_cfg->team_names[i]);
        }
    }
    return true;
}
