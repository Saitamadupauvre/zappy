#include "arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define ASSERT_TEST(cond, message) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s\n", message); \
            return false; \
        } \
    } while (0)

bool test_arena_creation_and_free(void) {
    size_t capacity = 1024; // 1KB
    arena_t arena = create_mem_arena(capacity);

    ASSERT_TEST(arena.base != NULL, "Arena initialization failed to allocate base pool.");
    ASSERT_TEST(arena.capacity == capacity, "Arena capacity initialization mismatch.");
    ASSERT_TEST(arena.used == 0, "New arena should initialize with zero bytes used.");

    free_mem_arena(&arena);
    ASSERT_TEST(arena.base == NULL, "Arena base pointer not zeroed after free.");
    ASSERT_TEST(arena.capacity == 0, "Arena capacity not reset to zero after free.");
    ASSERT_TEST(arena.used == 0, "Arena used counter not reset to zero after free.");

    return true;
}

bool test_arena_allocation_and_alignment(void) {
    arena_t arena = create_mem_arena(1024);

    // Test a standard allocation
    void *ptr1 = alloc_mem_arena(&arena, 4);
    ASSERT_TEST(ptr1 != NULL, "Failed to allocate 4 bytes from arena.");
    // Even though we requested 4 bytes, ALIGN_8 should force the tracker to jump by 8
    ASSERT_TEST(arena.used == 8, "Arena alignment tracking failed on 4-byte request.");

    // Test that the next allocation starts exactly where the previous aligned block ended
    void *ptr2 = alloc_mem_arena(&arena, 8);
    ASSERT_TEST(ptr2 == (uint8_t *)ptr1 + 8, "Subsequent allocation pointer calculation misaligned.");
    ASSERT_TEST(arena.used == 16, "Arena total usage calculation tracking error.");

    free_mem_arena(&arena);
    return true;
}

bool test_arena_out_of_memory(void) {
    arena_t arena = create_mem_arena(16); // Tiny arena exactly big enough for 2 alignment blocks

    void *ptr1 = alloc_mem_arena(&arena, 8);
    ASSERT_TEST(ptr1 != NULL, "Failed initial allocation within safe bounds.");

    // This request takes us exactly to capacity limit (16 bytes)
    void *ptr2 = alloc_mem_arena(&arena, 5); 
    ASSERT_TEST(ptr2 != NULL, "Failed edge-capacity allocation.");
    ASSERT_TEST(arena.used == 16, "Arena failed to hit perfect maximum capacity tracking.");

    // This allocation should overflow the capacity and safely return NULL
    void *ptr3 = alloc_mem_arena(&arena, 1);
    ASSERT_TEST(ptr3 == NULL, "Arena failed to catch Out of Memory condition and overflowed base boundary!");

    free_mem_arena(&arena);
    return true;
}

bool test_arena_reset(void) {
    arena_t arena = create_mem_arena(512);

    alloc_mem_arena(&arena, 64);
    alloc_mem_arena(&arena, 32);
    ASSERT_TEST(arena.used > 0, "Arena usage counter failed to increment.");

    reset_mem_arena(&arena);
    ASSERT_TEST(arena.used == 0, "Resetting arena failed to clear used tracker back to 0.");
    ASSERT_TEST(arena.base != NULL, "Resetting arena incorrectly altered base memory pointer block.");

    // Ensure we can re-allocate over the exact same space seamlessly now
    void *ptr = alloc_mem_arena(&arena, 16);
    ASSERT_TEST(ptr == arena.base, "Allocation post-reset did not return to the origin address pointer.");

    free_mem_arena(&arena);
    return true;
}
