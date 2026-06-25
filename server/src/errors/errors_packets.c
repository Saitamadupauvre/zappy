
#include "errors.h"


const char *packet_errors_to_str(PacketErrors e)
{
    static const char *messages[] = {
        [CONNECTION_CLOSED] = "Connection closed",
        [RECV_FAILED]       = "Receive failed",
        [WRONG_PROTOCOL_FORMAT] = "Wrongly formatted packet",
    };

    if (e < 0 || e >= PACKET_ERROR_COUNT) { return "Unknown error"; }
    return messages[e];
}
