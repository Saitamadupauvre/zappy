
#include "config.h"
#include "server.h"
#include "world.h"
#include "commands.h"


int handle_resource_respawn(server_t *server)
{
    static long last_respawn = 0;
    long now = now_ms();
    long interval_ms = (RESPAWN_INTERVAL * 1000L) / server->prog_cfg->frequency;

    if (interval_ms < 1) { interval_ms = 1; }
    if (last_respawn == 0) { last_respawn = now; }
    if (now - last_respawn < interval_ms) { return 0; }

    last_respawn = now;
    refill_world_resources(server->prog_cfg->world_map);

    return handle_mct(server, NULL);
}
