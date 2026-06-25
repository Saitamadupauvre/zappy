
#ifndef ARENA_H
    #define ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ALIGN_8(size) (((size) + 7) & ~7)

typedef struct arena_s {
    uint8_t *base;
    size_t used;
    size_t capacity;
} arena_t;

arena_t create_mem_arena(size_t input_capacity);

void *alloc_mem_arena(arena_t *mem_arena, size_t input_size);
void reset_mem_arena(arena_t *mem_arena);
void free_mem_arena(arena_t *mem_arena); 

#endif
