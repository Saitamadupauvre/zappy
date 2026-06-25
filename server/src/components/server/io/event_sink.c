
#include "event_sink.h"
#include "server.h"

#include <stdio.h>

#define FUNC "event_sink"


event_sink_t make_event_sink(server_t *server)
{
    event_sink_t sink = { .server = server };

    return sink;
}

int sink_to_fd(event_sink_t *sink, int fd, const char *msg, int len)
{
    (void)sink;
    return server_send(fd, msg, len, FUNC);
}

int sink_to_player(event_sink_t *sink, client_t *player,
    const char *msg, int len)
{
    return sink_to_fd(sink, player->fd, msg, len);
}

int sink_to_guis(event_sink_t *sink, const char *msg, int len)
{
    return group_gui_send(sink->server, msg, len, FUNC);
}

int sink_player_pos(event_sink_t *sink, client_t *player)
{
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "ppo #%d %d %d %d\n",
        player->fd,
        player->pos.x,
        player->pos.y,
        player->dir + 1); // ENUM + 1 to match protocol

    if (len < 0 || len >= (int)sizeof(buffer)) {
        fprintf(stderr, "[%s] Error: snprintf failed.\n", FUNC);
        return -1;
    }

    return sink_to_guis(sink, buffer, len);
}
