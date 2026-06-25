
#ifndef MATH_H
    #define MATH_H


    #include "server.h"

    #define TAN_NUM         414
    #define TAN_DEN         1000


void get_dir_vectors(Direction dir, int *fdx, int *fdy, int *rdx, int *rdy);

int wrap_delta(int delta, int size);
int sector_from_local(int f, int r);
int compute_direction(server_t *server, client_t *emitter, client_t *receiver);

#endif
