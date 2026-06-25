
#include "arena.h"
#include "server.h"


int group_gui_send(server_t *server, const char *message, int message_len, const char *from)
{
    for (size_t i = 0; i < server->guis.size; i++) {
        if (server_send(server->guis.vdata[i].fd, message, message_len, from) != 0) {
            return -1;
        }
    }

    return 0;
}
