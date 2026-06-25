
#include "errors.h"


const char *args_error_to_str(ArgsErrors e)
{
    static const char *messages[] = {
        [NOT_ENOUGH_ARGS]          = "Not enough arguments",
        [INVALID_PORT_ARG]         = "Invalid port argument",
        [INVALID_WIDTH_ARG]        = "Invalid width argument",
        [INVALID_HEIGHT_ARG]       = "Invalid height argument",
        [INVALID_TEAM_COUNT]       = "Invalid team count",
        [USAGE_OF_RESERVED_TEAM_NAME] = "<GRAPHIC> is reserved for the spectator GUI",
        [INVALID_NUMBER_OF_CLIENTS] = "Invalid number of clients",
        [INVALID_FREQUENCY]        = "Invalid frequency",
        [ARGS_PARSING_FAILED]      = "Argument parsing failed",
        [INVALID_PORT_RANGE]       = "Port out of range",
        [NON_INT_VALUE]            = "Expected an integer value",
        [INVALID_ARG_FORMAT]       = "Invalid argument format",
        [MISSING_MANDATORY_FLAG]   = "Missing one of the mandatory flags, every flag is needed except <-f> !",
        [UNKNOWN_ARG]              = "Unknown argument",
    };

    if (e < 0 || e >= ARGS_ERROR_COUNT) { return "Unknown error"; }
    return messages[e];
}
