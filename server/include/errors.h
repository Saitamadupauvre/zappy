
#ifndef ERRORS_H
    #define ERRORS_H

typedef enum {
    NOT_ENOUGH_ARGS,
    INVALID_PORT_ARG,
    INVALID_WIDTH_ARG,
    INVALID_HEIGHT_ARG,
    INVALID_TEAM_COUNT,
    USAGE_OF_RESERVED_TEAM_NAME,
    INVALID_NUMBER_OF_CLIENTS,
    INVALID_FREQUENCY,
    ARGS_PARSING_FAILED,
    INVALID_PORT_RANGE,
    NON_INT_VALUE,
    INVALID_ARG_FORMAT,
    UNKNOWN_ARG,
} args_errors_t;

typedef enum {
    STRUCT_ALLOC_FAILED,
    UNKNOWN_ALLOC_ERROR,
} allocation_errors_t;

const char *args_error_to_str(args_errors_t e);
const char *alloc_errors_to_str(allocation_errors_t e);

#endif
