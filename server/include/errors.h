
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
    MISSING_MANDATORY_FLAG,
    UNKNOWN_ARG,
    ARGS_ERROR_COUNT,
} ArgsErrors;

typedef enum {
    STRUCT_ALLOC_FAILED,
    TEAM_NAME_ALLOC_FAILED,
    COMMAND_ALLOC_FAILED,
    GUI_BROADCAST_BUFFER_FAILED,
    UNKNOWN_ALLOC_ERROR,
    ALLOC_ERROR_COUNT,
} AllocationErrors;

typedef enum {
    SOCKET_CREATION_FAILED,
    SOCKET_OPTIONS_FAILED,
    SOCKET_BIND_FAILED,
    SOCKET_LISTEN_FAILED,
    SERVER_SEND_FAILED,
    UNKNOWN_SERVER_ERROR,
    SERVER_ERROR_COUNT,
} ServerErrors;


#define CONNECTION_CLOSED_VAL -1
#define RECV_FAILED_VAL -2
#define WRONG_PROTOCOL_FORMAT_VAL -3

typedef enum {
    CONNECTION_CLOSED,
    RECV_FAILED,
    WRONG_PROTOCOL_FORMAT,
    PACKET_ERROR_COUNT,
} PacketErrors;

const char *args_error_to_str(ArgsErrors e);
const char *alloc_errors_to_str(AllocationErrors e);
const char *server_errors_to_str(ServerErrors e);
const char *packet_errors_to_str(PacketErrors e);


int print_server_errors(ServerErrors err, const char *func);

#endif
