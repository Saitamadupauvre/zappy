
#include "server.h"
#include "commands.h"
#include "world.h"
#include "event_sink.h"

#include <stdio.h>
#include <string.h>


int handle_inventory(server_t *server, client_t *client)
{
    event_sink_t sink = make_event_sink(server);
    int needed = 3;
    for (int i = 0; i < RESOURCE_COUNT; i++)
        needed += snprintf(NULL, 0, "%s%s %d",
            i > 0 ? ", " : "", ressources_to_str((Ressources)i), client->inv[i]);

    char buf[needed + 1];
    int pos = 0;

    buf[pos++] = '[';
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        int written = snprintf(buf + pos, needed - pos,
            "%s%s %d", i > 0 ? ", " : "", ressources_to_str((Ressources)i), client->inv[i]);
        if (written < 0 || written >= needed - pos) { return -1; }
        pos += written;
    }
    buf[pos++] = ']';
    buf[pos++] = '\n';

    client->cd = set_client_cd(server->prog_cfg->frequency, CD_INVENTORY);

    return sink_to_player(&sink, client, buf, pos);
}
