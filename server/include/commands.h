
#ifndef COMMANDS_H
    #define COMMANDS_H


    #include "server.h"
#include "world.h"

    #define MAX_COMMANDS_COUNT 10
    #define UNKNOWN_COMMAND "ko\n"
    #define COMMAND_PARAM "ko\n"
    #define DEAD_MSG "dead\n"
    #define COMMAND_OK "ok\n"

    #define CD_FORWARD      7
    #define CD_RIGHT        7
    #define CD_LEFT         7
    #define CD_LOOK         7
    #define CD_INVENTORY    1
    #define CD_BROADCAST    7
    #define CD_EJECT        7
    #define CD_TAKE         7
    #define CD_SET          7
    #define CD_FORK         42
    #define CD_INCANTATION  300

typedef enum {
    MSZ,
    BCT,
    MCT,
    TNA,
    PPO,
    PLV,
    PIN,
    SGT,
    SST,
    STU,
    CMD_UNKNOWN
} GuiCommands;

typedef enum {
    FORWARD,
    RIGHT,
    LEFT,
    LOOK,
    INVENTORY,
    BROADCAST,
    CONNECT_NBR,
    FORK,
    EJECT,
    TAKE_OBJ,
    SET_OBJ,
    INCANTATION,
    COMMAND_UNKNOWN,
} PlayerCommands;

typedef int (*gui_handle_single)(server_t *server, client_t *client);
typedef int (*gui_handle_args)(server_t *server, client_t *client, char *args);
typedef int (*gui_handle_custom)(server_t *server);

typedef int (*player_handle_single)(server_t *server, client_t *client);
typedef int (*player_handle_args)(server_t *server, client_t *client, char *args);

typedef struct gui_command_info_s {
    const char *cmd_name;
    GuiCommands cmd_type;
    bool has_args;
    gui_handle_single func;
    gui_handle_args func_args;
    gui_handle_custom func_custom;
} gui_command_info_t;

typedef struct player_command_info_s {
    const char *cmd_name;
    PlayerCommands cmd_type;
    bool has_args;
    player_handle_single func;
    player_handle_args func_args;
} player_command_info_t;

int push_commands(client_t *client, char *command);
void pop_commands(client_t *client, int idx);

// PLAYER COMMANDS

int handle_player_commands(server_t *server, client_t *client, char *command);

int handle_forward(server_t *server, client_t *client);
int handle_right(server_t *server, client_t *client);
int handle_left(server_t *server, client_t *client);
int handle_look(server_t *server, client_t *client);
int handle_inventory(server_t *server, client_t *client);
int handle_broadcast(server_t *server, client_t *client, char *args);
int handle_eject(server_t *server, client_t *client);
int handle_take(server_t *server, client_t *client, char *args);
int handle_set(server_t *server, client_t *client, char *args);
int handle_connect_nbr(server_t *server, client_t *client);
int handle_fork(server_t *server, client_t *client);
int handle_incantation(server_t *server, client_t *client);
int finish_incantation(server_t *server, client_t *client);

// GUI COMMANDS

int handle_gui_commands(server_t *server, client_t *client, char *command);

int handle_msz(server_t *server, client_t *client);
int handle_mct(server_t *server, client_t *client);
int handle_tna(server_t *server, client_t *client);
int handle_sgt(server_t *server, client_t *client);
int handle_bct(server_t *server, client_t *client, char *args);
int handle_sst(server_t *server, client_t *client, char *arg);
int handle_ppo(server_t *server, client_t *client, char *arg);
int handle_plv(server_t *server, client_t *client, char *arg);
int handle_pin(server_t *server, client_t *client, char *arg);
int handle_stu(server_t *server);

// PASSIVE COMMANDS

int handle_pnw(server_t *server, client_t *client);
int notify_gui_ppo(server_t *server, client_t *client);
int handle_pex(client_t *client, int victim_fd);
int handle_pdi(server_t *server, int player_fd);
int handle_pbc(server_t *server, client_t *client, const char *message);
int handle_pie(server_t *server, client_t *client, bool inc_res);
int handle_pic(server_t *server, client_t *client);
int handle_pfk(server_t *server, client_t *client);
int handle_pdr(server_t *server, client_t *client, Ressources type);
int handle_pgt(server_t *server, client_t *client, Ressources type);

int handle_enw(server_t *server, client_t *client, int egg_id);
int handle_ebo(server_t *server, int egg_id);
int handle_edi(server_t *server, int egg_id);
int handle_seg(server_t *server, const char *team_name);
int handle_smg(server_t *server, const char *message);

#endif
