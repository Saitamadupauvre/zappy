
#include "server.h"
#include "world.h"

#include <string.h>


#define GUI_STR "GUI"
#define PLAYER_STR "PLAYER"
#define UNKNOWN_STR "UNKNOWN"

#define NORTH_STR "North"
#define EAST_STR "East"
#define SOUTH_STR "South"
#define WEST_STR "West"

#define FOOD_STR "food"
#define LINEMATE_STR "linemate"
#define DERAUMERE_STR "deraumere"
#define SIBUR_STR "sibur"
#define MENDIANE_STR "mendiane"
#define PHIRAS_STR "phiras"
#define THYSTAME_STR "thystame"


const char *ressources_to_str(Ressources type)
{
    switch (type) {
        case FOOD:      return FOOD_STR;
        case LINEMATE:  return LINEMATE_STR;
        case DERAUMERE: return DERAUMERE_STR;
        case SIBUR:     return SIBUR_STR;
        case MENDIANE:  return MENDIANE_STR;
        case PHIRAS:    return PHIRAS_STR;
        case THYSTAME:  return THYSTAME_STR;
        default:        return UNKNOWN_STR;
    }
}

const char *type_to_str(ClientType ct)
{
    switch (ct) {
        case GUI: return GUI_STR;
        case PLAYER: return PLAYER_STR;
        default: return UNKNOWN_STR; 
    }
}

int str_to_ressource(const char *str)
{
    for (int i = 0; i < RESOURCE_COUNT; i++) {
        if (strcmp(str, ressources_to_str((Ressources)i)) == 0) {
            return i;
        }
    }

    return -1;
}

const char *direction_to_str(Direction dir)
{
    switch (dir) {
        case NORTH: return NORTH_STR;
        case EAST: return EAST_STR;
        case SOUTH: return SOUTH_STR;
        case WEST: return WEST_STR;
        default: return UNKNOWN_STR; 
    }
}
