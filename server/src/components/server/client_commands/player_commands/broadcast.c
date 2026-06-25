
#include "arena.h"
#include "server.h"
#include "commands.h"
#include "math.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>

#define BROADCAST_OK    "ok\n"


static int send_message(event_sink_t *sink, server_t *server, client_t *emitter,
    client_t *receiver, const char *text)
{
    size_t buffer_len = 20 + strlen(text) + 1;
    arena_t arena = create_mem_arena(buffer_len);
    char *buf = alloc_mem_arena(&arena, buffer_len);
    if (!buf) {
        free_mem_arena(&arena);
        return -1;
    }

    int dir = compute_direction(server, emitter, receiver);
    int len = snprintf(buf, buffer_len, "message %d, %s\n", dir, text);
    if (len < 0 || len >= (int)buffer_len) {
        free_mem_arena(&arena);
        return -1;
    }

    int ret = sink_to_fd(sink, receiver->fd, buf, len);
    free_mem_arena(&arena);
    return ret;
}

static bool players_can_hear(client_t *client)
{
    return client->type == PLAYER && client->cstate == CONN
        && (client->state == ALIVE || client->state == CHANNELING);
}

int handle_broadcast(server_t *server, client_t *client, char *args)
{
    event_sink_t sink = make_event_sink(server);

    for (size_t i = 0; i < server->players.size; i++) {
        client_t *receiver = &server->players.vdata[i];
        if (receiver->fd == client->fd) { continue; }
        if (players_can_hear(receiver)) {
            if (send_message(&sink, server, client, receiver, args) != 0) {
                return -1;
            }
        }
    }

    if (handle_pbc(server, client, args) != 0) { return -1; } //dobules has the notify gui
    client->cd = set_client_cd(server->prog_cfg->frequency, CD_BROADCAST);

    return sink_to_player(&sink, client, BROADCAST_OK, strlen(BROADCAST_OK));
}
