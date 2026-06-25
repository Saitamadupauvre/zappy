
#ifndef SERVER_H
    #define SERVER_H

    #include "arena.h"
#include "config.h"
    #include "world.h"
    #include "vector.h"

    #include <stdbool.h>
    #include <sys/types.h>

    #define MAX_COMMANDS 10
    #define RECEPTION_SIZE 4096

typedef enum {
    GUI,
    PLAYER,
} ClientType;

typedef enum {
    PENDING,
    CONN,
    REJECTED,
} ClientState;

typedef enum {
    EGG,
    ALIVE,
    CHANNELING,
    DEAD,
} PlayerState;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST,
} Direction;

typedef struct team_info_s {
    const char *team_name;
    int player_count;
    int egg_count;
    int team_slots;
    int max_lvls;
} team_info_t;

typedef struct egg_s {
    int egg_id;
    int player_fd;
    int team_idx;
    int x;
    int y;
} egg_t;

typedef struct cmd_queue_s {
    char *cmd[MAX_COMMANDS];
    int cmd_count;
} cmd_queue_t;

typedef struct client_pos_s {
    int x;
    int y;
} client_pos_t;

typedef struct client_s {
    ClientState cstate;
    ClientType type;
    PlayerState state;
    client_pos_t pos;
    cmd_queue_t commands;
    Direction dir;
    arena_t arena;
    int fd;
    int inv[RESOURCE_COUNT];
    int cd;
    int lvl;
    int team_idx;
    long food_dealine;
    char *team_name;
    long action_deadline;
    char recept_buffer[RECEPTION_SIZE];
    size_t recv_stored;
    bool incant_initiator;
} client_t;

typedef struct server_s {
    prog_cfg_t *prog_cfg;
    team_info_t *teams;
    vec_client_t homeless;
    vec_client_t players;
    vec_client_t guis;
    vec_egg_t eggs;
    vec_pollfd_t pollds;
    int next_egg_id;
    int client_count;
    int curr_connected;
    int server_fd;
    bool game_over;
    unsigned long uptime;
} server_t;


// BASE SERVER FUNCS

server_t *init_server(prog_cfg_t *prog_cfg);
int run_server(prog_cfg_t *prog_cfg, server_t *server);

// GAME TICK (see components/game/game.c) -- single simulation step.
// Returns 1 when the game is over, 0 otherwise.
int game_tick(server_t *server);

client_t init_client(int client_fd);
struct pollfd init_polls(int handle_fd);

bool handle_win_con(server_t *server);
void server_cleanup(server_t *server);
void server_shutdown(server_t *server);

int setup_signal_handlers(void);
int shutdown_requested(void);

// PLAYER FUNCS

int handle_player_stats(server_t *server);

// WORLD FUNCS

int handle_resource_respawn(server_t *server);

// WRAPPERS AND HELPERS

int server_send(int target_fd, const char *content, int content_len, const char *from);
ssize_t recv_parser(client_t *client, char **command);
void disconnect_client(server_t *server, size_t poll_idx);
client_t *find_client(server_t *server, int fd);
void set_client_type(client_t *client, char *cmd);

map_tile_t *get_tile(server_t *server, int x, int y);

const char *type_to_str(ClientType ct);
const char *direction_to_str(Direction dir);
const char *ressources_to_str(Ressources type);
int str_to_ressource(const char *str);
int set_client_cd(int freq, int cd);
int parse_gui_id(char *arg);

char *get_args(char *command, size_t cmd_len);

int group_gui_send(server_t *server, const char *message, int message_len, const char *from);

// CLIENT CONN

int handle_client_connection(server_t *server, client_t *client, char *received_input);

// Handshae related func

int handle_client_handshake(server_t *server, client_t *client, char *team);

int find_team(server_t *server, const char *team);
int check_team_slot(server_t *server, int team_idx);
int reject_client(client_t *client, const char *reason);

 // FOR COMMANDS LOOK IN commands.h
 
int handle_commands_dispatch(server_t *server);

#endif
