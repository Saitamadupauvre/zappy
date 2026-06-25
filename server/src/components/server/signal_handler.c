
#include "server.h"

#include <signal.h>
#include <stdio.h>

static volatile sig_atomic_t g_shutdown_requested = 0;

static void shutdown_signal_handler(int signum)
{
    (void)signum;
    g_shutdown_requested = 1;
}

int shutdown_requested(void)
{
    return g_shutdown_requested != 0;
}

int setup_signal_handlers(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = shutdown_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "Error: failed to install SIGINT handler\n");
        return -1;
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Error: failed to install SIGTERM handler\n");
        return -1;
    }

    // Ignore SIGPIPE: writing to a client that just disconnected (e.g. a
    // player that died/quit) must make send() return EPIPE for server_send
    // to handle, not kill the whole server with the default SIGPIPE action.
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        fprintf(stderr, "Error: failed to ignore SIGPIPE\n");
        return -1;
    }

    return 0;
}
