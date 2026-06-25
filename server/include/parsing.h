/*
** EPITECH PROJECT, 2025
** B-CPE-200-MPL-2-1-corewar-frederick.jeffcock
** File description:
** parsing.h
*/

#ifndef PARSING_H
    #define PARSING_H

    #include "errors.h"
    #include <stdbool.h>

typedef struct prog_cfg_s prog_cfg_t;
typedef struct args_list_s args_list_t;


bool parse_ints(char *integer, int *cfg_field, bool *arg_check, ArgsErrors error_type);
bool parse_port(char *port, prog_cfg_t *prog_cfg, bool *arg_check);
bool parse_teams(int argc, char *argv[], prog_cfg_t *prog_cfg, bool *arg_check);
bool parse_invalid_flag(char arg);

#endif
