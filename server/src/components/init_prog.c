
#include "config.h"
#include "errors.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static void free_mem(prog_cfg_t *prog_cfg)
{
    if (prog_cfg->team_names != NULL) {
        for (int i = 0; prog_cfg->team_names[i] != NULL; i++) {
            free(prog_cfg->team_names[i]);
            prog_cfg->team_names[i] = NULL;
        }
        free(prog_cfg->team_names);
        prog_cfg->team_names = NULL;
    }
    free(prog_cfg);
}

int handle_pre_serv_proc(int argc, char *argv[])
{
    if (argc == 2 && strncmp(argv[1], HELP_FLAG, strlen(HELP_FLAG)) == 0) {
        printf("%s\n", USAGE);
        return 0;
    }

    prog_cfg_t *prog_cfg = malloc(sizeof(prog_cfg_t));
    if (prog_cfg == NULL) {
        fprintf(stderr, "Error: %s, for prog_cfg\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return -1;
    }
    memset(prog_cfg, 0, sizeof(prog_cfg_t));

    if (!parse_cmd_line(argc, argv, prog_cfg)) {
        free_mem(prog_cfg);
        fprintf(stderr, "Error: %s.\n", args_error_to_str(ARGS_PARSING_FAILED));
        return -1;
    }

    if (prog_cfg != NULL) { free_mem(prog_cfg); }
    return 0;
}
