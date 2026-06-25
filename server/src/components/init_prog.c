
#include "config.h"
#include "errors.h"
#include "arena.h"
#include "world.h"
#include "server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define INIT_WORLD_FAILURE "Error: failed to init world"
#define INIT_SERVER_FAILURE "Error: failed to init server"
#define RUN_SERVER_FAILURE "Error: Server broke"


static int handle_failure(prog_cfg_t *prog_cfg, const char *msg)
{
    fprintf(stderr, "Error: %s.\n", msg);
    free_mem_arena(&prog_cfg->perm_mem_arena);
    return -1;
}

int handle_pre_serv_proc(int argc, char *argv[])
{
    if (argc == 2 && strncmp(argv[1], HELP_FLAG, strlen(HELP_FLAG)) == 0) {
        printf("%s\n", USAGE);
        return 0;
    }

    prog_cfg_t prog_cfg = {0};
    prog_cfg.perm_mem_arena = create_mem_arena(PERM_ARENA_SIZE);
    if (prog_cfg.perm_mem_arena.base == NULL) {
        fprintf(stderr, "Error: %s, for prog_cfg\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED));
        return -1;
    }

    if (!parse_cmd_line(argc, argv, &prog_cfg)) {
        free_mem_arena(&prog_cfg.perm_mem_arena);
        fprintf(stderr, "Error: %s.\n", args_error_to_str(ARGS_PARSING_FAILED));
        return -1;
    }

    srand(time(NULL));
    if (init_world_map(&prog_cfg) != 0) { return handle_failure(&prog_cfg, INIT_WORLD_FAILURE); }

    server_t *server = init_server(&prog_cfg);
    if (server == NULL) { return handle_failure(&prog_cfg, INIT_SERVER_FAILURE); }
    if (run_server(&prog_cfg, server) != 0) { return handle_failure(&prog_cfg, RUN_SERVER_FAILURE); }

    free_mem_arena(&prog_cfg.perm_mem_arena);
    return 0;
}
