
#ifndef EVENT_SINK_H
    #define EVENT_SINK_H

    #include "server.h"


typedef struct event_sink_s {
    server_t *server;
} event_sink_t;

event_sink_t make_event_sink(server_t *server);

int sink_to_fd(event_sink_t *sink, int fd, const char *msg, int len);
int sink_to_player(event_sink_t *sink, client_t *player, const char *msg, int len);
int sink_to_guis(event_sink_t *sink, const char *msg, int len);
int sink_player_pos(event_sink_t *sink, client_t *player);

#endif
