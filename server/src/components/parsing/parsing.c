
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

static bool valide_needed_flags(args_list_t *args_list)
{
    if (!args_list->has_port || !args_list->has_width || !args_list->has_height ||
        !args_list->has_clients || !args_list->has_teams) {
        fprintf(stderr, "Error: %s.\n", args_error_to_str(MISSING_MANDATORY_FLAG));
        return false;
    }

    return true;
}

bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg)
{
    args_list_t args = {0};
    prog_cfg->frequency = DEFAULT_FREQUENCY;
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

    if (!valide_needed_flags(&args)) { return false; }

    return true;
}
