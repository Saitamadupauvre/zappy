
#include "errors.h"
#include <stdio.h>


const char *server_errors_to_str(ServerErrors e)
{
    static const char *messages[] = {
        [SOCKET_CREATION_FAILED] = "Server socket failed to initialize",
        [SOCKET_OPTIONS_FAILED]  = "Server socket options failed",
        [SOCKET_BIND_FAILED]     = "Server socket bind failed",
        [SOCKET_LISTEN_FAILED]   = "Server socket listen failed",
        [SERVER_SEND_FAILED]     = "Server send failed",
        [UNKNOWN_SERVER_ERROR]   = "Unknown server error",
    };

    if (e < 0 || e >= SERVER_ERROR_COUNT) { return "Unknown error"; }
    return messages[e];
}

int print_server_errors(ServerErrors err, const char *func)
{
    fprintf(stderr, "Error: %s in %s.\n", server_errors_to_str(err), func);
    return -1;
}
