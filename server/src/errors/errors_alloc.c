
#include "errors.h"

#include <stdio.h>





const char *alloc_errors_to_str(allocation_errors_t e)
{
    static const char *messages[] = {
        [STRUCT_ALLOC_FAILED]  = "Structure allocation failed",
        [UNKNOWN_ALLOC_ERROR]  = "Unknown allocation error",
    };

    if (e < 0 || e >= (allocation_errors_t)(sizeof(messages) / sizeof(*messages)) || !messages[e]) { return "Unknown error";}
    return messages[e];
}