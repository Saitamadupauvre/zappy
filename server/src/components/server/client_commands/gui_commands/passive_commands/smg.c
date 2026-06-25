
#include "arena.h"
#include "server.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>


int handle_smg(server_t *server, const char *message)
{
    if (message == NULL) { return -1; }

    size_t buffer_size = 8 + strlen(message) + 1;
    arena_t buffer_arena = create_mem_arena(buffer_size);
    char *buffer = alloc_mem_arena(&buffer_arena, buffer_size);
    if (buffer == NULL) {
        fprintf(stderr, "[handle_smg] Error: arena alloc failed.\n");
        free_mem_arena(&buffer_arena);
        return -1;
    }

    int fmt_len = snprintf(buffer, buffer_size, "smg %s\n", message);
    if (fmt_len < 0 || fmt_len >= (int)buffer_size) {
        fprintf(stderr, "[handle_smg] Error: snprintf failed.\n");
        free_mem_arena(&buffer_arena);
        return -1;
    }

    int ret = group_gui_send(server, buffer, fmt_len, "handle_smg");
    free_mem_arena(&buffer_arena);
    return ret;
}
