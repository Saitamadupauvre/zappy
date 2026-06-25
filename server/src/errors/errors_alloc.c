
#include "errors.h"


const char *alloc_errors_to_str(AllocationErrors e)
{
    static const char *messages[] = {
        [STRUCT_ALLOC_FAILED]  = "Structure allocation failed",
        [TEAM_NAME_ALLOC_FAILED] = "Team name to client allocation failed",
        [COMMAND_ALLOC_FAILED] = "Command allocation failed in flush buffer",
        [GUI_BROADCAST_BUFFER_FAILED] = "Broadcast buffer allocation failed",
        [UNKNOWN_ALLOC_ERROR]  = "Unknown allocation error",
    };

    if (e < 0 || e >= ALLOC_ERROR_COUNT) { return "Unknown error"; }
    return messages[e];
}
