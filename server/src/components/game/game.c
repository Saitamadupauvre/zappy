
#include "server.h"


int game_tick(server_t *server)
{
    handle_player_stats(server);
    handle_commands_dispatch(server);
    if (handle_win_con(server)) { return 1; }
    handle_resource_respawn(server);

    server->uptime += 1;

    return 0;
}
