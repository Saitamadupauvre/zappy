
#include "errors.h"

#include <stdio.h>





const char *args_error_to_str(args_errors_t e)
{
    static const char *messages[] = {
        [NOT_ENOUGH_ARGS]          = "Not enough arguments",
        [INVALID_PORT_ARG]         = "Invalid port argument",
        [INVALID_WIDTH_ARG]        = "Invalid width argument",
        [INVALID_HEIGHT_ARG]       = "Invalid height argument",
        [INVALID_TEAM_COUNT]       = "Invalid team count",
        [USAGE_OF_RESERVED_TEAM_NAME] = "<GRAPHIC> is reserved for the spectator GUI",
        [INVALID_NUMBER_OF_CLIENTS]= "Invalid number of clients",
        [INVALID_FREQUENCY]        = "Invalid frequency",
        [ARGS_PARSING_FAILED]      = "Argument parsing failed",
        [INVALID_PORT_RANGE]       = "Port out of range",
        [NON_INT_VALUE]            = "Expected an integer value",
        [INVALID_ARG_FORMAT]       = "Invalid argument format",
        [UNKNOWN_ARG]              = "Unknown argument",
    };

    if (e < 0 || e >= (args_errors_t)(sizeof(messages) / sizeof(*messages)) || !messages[e]) { return "Unknown error"; }

    return messages[e];
}
