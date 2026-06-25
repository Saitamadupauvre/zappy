
#include "arena.h"
#include "config.h"
#include "errors.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>

#define BANNED_NAME "GRAPHIC"

bool parse_invalid_flag(char arg)
{ 
    if (arg == 'p' || arg == 'x' || arg == 'y' || arg == 'c' || arg == 'f') {
        fprintf(stderr, "Error [%c]: %s .\n", arg,args_error_to_str(INVALID_ARG_FORMAT));
        return false;
    } else {
        fprintf(stderr, "Error: %s.\n", args_error_to_str(UNKNOWN_ARG)); 
        return false;
    }
}

static bool check_dups(char *team_names[], int team_count)
{
    for (int i = 0; i < team_count; i++) {
        for (int y = i + 1; y < team_count; y++) {
           if (strcmp(team_names[i], team_names[y]) == 0) {
               return false;
           } 
        }
    }

    return true;
}

static bool valide_team_name(char *argv[], int team_idx, int team_count)
{
    for (int x = 0; x < team_count; x++) {
        if (strcmp(argv[team_idx + x], BANNED_NAME) == 0) { 
            return false;
        }
    }

    return true;
}

bool parse_teams(int argc, char *argv[], prog_cfg_t *prog_cfg, bool *arg_check)
{
    int team_start = optind;

    for (; optind < argc && argv[optind][0] != '-'; optind++) {
        prog_cfg->team_count++;
    }
    
    if (prog_cfg->team_count <= 0) {
        fprintf(stderr, "Error: %s.\n", args_error_to_str(INVALID_TEAM_COUNT));
        return false;
    }

    if (!valide_team_name(argv, team_start, prog_cfg->team_count)) { 
        fprintf(stderr, "Error: %s.\n", args_error_to_str(USAGE_OF_RESERVED_TEAM_NAME));
        return false;
    }

    size_t team_names_len = (sizeof(char *) * (prog_cfg->team_count + 1)); 
    prog_cfg->team_names = alloc_mem_arena(&prog_cfg->perm_mem_arena, team_names_len);
    if (prog_cfg->team_names == NULL) {
        fprintf(stderr, "Error: allocation memory for team names.\n");
        return false;
    }
    prog_cfg->team_names[prog_cfg->team_count] = NULL;

    for (int i = 0; i < prog_cfg->team_count; i++) {
        size_t name_len = strlen(argv[team_start + i]) + 1;
        prog_cfg->team_names[i] = alloc_mem_arena(&prog_cfg->perm_mem_arena, name_len);
        if (prog_cfg->team_names[i] == NULL) {
            fprintf(stderr, "Error: %s.\n", alloc_errors_to_str(STRUCT_ALLOC_FAILED)); 
            return false;
        }
        memcpy(prog_cfg->team_names[i], argv[team_start + i], name_len);
    }

    if (!check_dups(prog_cfg->team_names, prog_cfg->team_count)) { return false; }

    *arg_check = true;
    return true;
}

bool parse_ints(char *integer, int *cfg_field, bool *arg_check, ArgsErrors error_type)
{
    errno = 0;
    char *endptr = NULL;
    long integer_val = strtol(integer, &endptr, 10);

    if (*integer == '\0' || *endptr != '\0') {
        fprintf(stderr, "Error: %s\n", args_error_to_str(error_type));
        return false;
    }

    if (errno == ERANGE || integer_val <= 0) {
        fprintf(stderr, "Error: Value out of range or invalid for flag.\n");
        return false;
    }

    *cfg_field = integer_val;
    *arg_check = true;
    return true;
}

bool parse_port(char *port, prog_cfg_t *prog_cfg, bool *arg_check)
{
    errno = 0;
    char *endptr = NULL;
    long port_val = strtol(port, &endptr, 10);

    if (*port == '\0' || *endptr != '\0') {
        fprintf(stderr, "Error: %s\n", args_error_to_str(INVALID_PORT_ARG));
        return false;
    }

    if (errno == ERANGE) {
        fprintf(stderr, "Error: %d\n", errno);
        return false;
    }

    if (port_val < PORT_MIN || port_val > PORT_MAX) {
        fprintf(stderr, "Error: %s\n", args_error_to_str(INVALID_PORT_RANGE));
        return false;
    }

    prog_cfg->port = (uint16_t)port_val;
    *arg_check = true;
    return true;
}
