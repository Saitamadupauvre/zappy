
#include "arena.h"

#include <stdlib.h>


arena_t create_mem_arena(size_t input_capacity)
{
    size_t aligned_capacity = ALIGN_8(input_capacity);
    arena_t mem_arena = {
        .base = malloc(aligned_capacity),
        .used = 0,
        .capacity = aligned_capacity,
    };

    return mem_arena;
}

void *alloc_mem_arena(arena_t *mem_arena, size_t input_size)
{
    size_t aligned_size = ALIGN_8(input_size);
    if (mem_arena->base == NULL || mem_arena->used + aligned_size > mem_arena->capacity) { return NULL; }
    void *offset_ptr = mem_arena->base + mem_arena->used;
    mem_arena->used += aligned_size;

    return offset_ptr;
}

void reset_mem_arena(arena_t *mem_arena)
{
    mem_arena->used = 0;
}

void free_mem_arena(arena_t *mem_arena)
{
    if (mem_arena->base == NULL) { return; }
    free(mem_arena->base);

    mem_arena->base = NULL;
    mem_arena->capacity = 0;
    mem_arena->used = 0;
}
