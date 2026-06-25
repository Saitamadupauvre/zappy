
#include "server.h"
#include "math.h"

#include <stdlib.h>


void get_dir_vectors(Direction dir, int *fdx, int *fdy, int *rdx, int *rdy)
{
    switch (dir) {
        case NORTH: *fdx =  0; *fdy = -1; *rdx =  1; *rdy =  0; break;
        case SOUTH: *fdx =  0; *fdy =  1; *rdx = -1; *rdy =  0; break;
        case EAST:  *fdx =  1; *fdy =  0; *rdx =  0; *rdy =  1; break;
        case WEST:  *fdx = -1; *fdy =  0; *rdx =  0; *rdy = -1; break;
    }
}

int wrap_delta(int delta, int size)
{
    delta %= size;
    if (delta > size / 2)  delta -= size;
    if (delta < -(size / 2)) delta += size;
    return delta;
}

int sector_from_local(int f, int r)
{
    long af = labs(f);
    long ar = labs(r);

    if (f == 0 && r == 0) return 0;
    if ((long)TAN_DEN * ar < (long)TAN_NUM * af) return f > 0 ? 1 : 5;
    if ((long)TAN_DEN * af < (long)TAN_NUM * ar) return r > 0 ? 7 : 3;
    if (f > 0) return r > 0 ? 8 : 2;
    return r > 0 ? 6 : 4;
}

int compute_direction(server_t *server, client_t *emitter,
    client_t *receiver)
{
    int fdx, fdy, rdx, rdy;
    int dx = wrap_delta(emitter->pos.x - receiver->pos.x, server->prog_cfg->w);
    int dy = wrap_delta(emitter->pos.y - receiver->pos.y, server->prog_cfg->h);

    get_dir_vectors(receiver->dir, &fdx, &fdy, &rdx, &rdy);
    return sector_from_local(dx * fdx + dy * fdy, dx * rdx + dy * rdy);
}
