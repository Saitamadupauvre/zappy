
#include "config.h"


int main(int argc, char *argv[])
{
    if (handle_pre_serv_proc(argc, argv) == -1) {
        return EXIT_FAILED;
    }
    return EXIT_SUCCESS;
}
