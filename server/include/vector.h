
#ifndef VECTOR_H
    #define VECTOR_H

    #include "arena.h"

    #include <stdlib.h>
    #include <poll.h>

typedef struct client_s client_t;
typedef struct egg_s egg_t;

typedef struct vec_egg_s {
    egg_t *vdata;
    size_t size;
    size_t capacity;
} vec_egg_t;

typedef struct vec_pollfd_s {
    struct pollfd *vdata;
    size_t size;
    size_t capacity;
} vec_pollfd_t;

typedef struct vec_client_s {
    client_t *vdata;
    size_t size;
    size_t capacity;
} vec_client_t;

// ── arena-backed (fixed capacity, no realloc) ──────────────────────────────

#define init_vector_arena(vec, cap, arena)                                  \
    do {                                                                    \
        (vec)->vdata    = alloc_mem_arena((arena),                          \
                            sizeof(*(vec)->vdata) * (cap));                 \
        (vec)->size     = 0;                                                \
        (vec)->capacity = (vec)->vdata ? (cap) : 0;                        \
    } while (0)

#define destroy_vector_arena(vec)                                           \
    do {                                                                    \
        (vec)->vdata    = NULL;                                             \
        (vec)->size     = 0;                                                \
        (vec)->capacity = 0;                                                \
    } while (0)

#define vector_pushb_arena(vec, val)                                        \
    ({                                                                      \
        int _res = 0;                                                       \
        if ((vec)->size >= (vec)->capacity) {                               \
            _res = -1;                                                      \
        } else {                                                            \
            (vec)->vdata[(vec)->size++] = (val);                            \
        }                                                                   \
        _res;                                                               \
    })

// ── shared ops (work on both arena and malloc backed) ─────────────────────

#define vector_popb(vec)                                                    \
    do {                                                                    \
        if ((vec)->size > 0)                                                \
            (vec)->size--;                                                  \
    } while (0)

#define vector_remove(vec, idx)                                             \
    do {                                                                    \
        if ((idx) < (vec)->size) {                                          \
            (vec)->vdata[(idx)] = (vec)->vdata[(vec)->size - 1];            \
            (vec)->size--;                                                  \
        }                                                                   \
    } while (0)

#define vector_at(vec, idx)     ((vec)->vdata[(idx)])
#define vector_size(vec)        ((vec)->size)
#define vector_full(vec)        ((vec)->size >= (vec)->capacity)
#define vector_empty(vec)       ((vec)->size == 0)

// ── malloc backed (for anything outside arena scope) ──────────────────────

#define init_vector(vec, initial_cap)                                       \
    do {                                                                    \
        (vec)->vdata = malloc(sizeof(*(vec)->vdata) * (initial_cap));       \
        if ((vec)->vdata == NULL) {                                         \
            (vec)->size     = 0;                                            \
            (vec)->capacity = 0;                                            \
        } else {                                                            \
            (vec)->size     = 0;                                            \
            (vec)->capacity = (initial_cap);                                \
        }                                                                   \
    } while (0)

#define destroy_vector(vec)                                                 \
    do {                                                                    \
        free((vec)->vdata);                                                 \
        (vec)->vdata    = NULL;                                             \
        (vec)->size     = 0;                                                \
        (vec)->capacity = 0;                                                \
    } while (0)

#define vector_pushb(vec, val)                                              \
    ({                                                                      \
        int _res = 0;                                                       \
        if ((vec)->size == (vec)->capacity) {                               \
            size_t _new_cap = (vec)->capacity == 0                         \
                ? 1                                                         \
                : (vec)->capacity * 2;                                      \
            typeof((vec)->vdata) _new_data = realloc(                      \
                (vec)->vdata, sizeof(*(vec)->vdata) * _new_cap);           \
            if (_new_data == NULL) {                                        \
                _res = -1;                                                  \
            } else {                                                        \
                (vec)->vdata    = _new_data;                                \
                (vec)->capacity = _new_cap;                                 \
            }                                                               \
        }                                                                   \
        if (_res == 0)                                                      \
            (vec)->vdata[(vec)->size++] = (val);                            \
        _res;                                                               \
    })


#endif
