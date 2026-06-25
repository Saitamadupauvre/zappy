
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Forward declarations of your parsing tests
extern bool test_valid_standard_args(void);
extern bool test_missing_mandatory_args(void);
extern bool test_default_frequency_fallback(void);
extern bool test_invalid_non_integer_strings(void);
extern bool test_out_of_bounds_port_ranges(void);
extern bool test_zero_and_negative_map_dimensions(void);
extern bool test_missing_team_names_after_flag(void);

// Forward declarations of your memory arena tests
extern bool test_arena_creation_and_free(void);
extern bool test_arena_allocation_and_alignment(void);
extern bool test_arena_out_of_memory(void);
extern bool test_arena_reset(void);

int main(void) {
    int passed = 0;
    int total = 0;

    printf("=========================================\n");
    printf("RUNNING ALL ZAPPY SERVER UNIT TESTS\n");
    printf("=========================================\n\n");

    // --- Command Line Parser Suite ---
    printf("[SUITE] Command Line Parsing\n");
    total++; if (test_valid_standard_args()) passed++;
    total++; if (test_missing_mandatory_args()) passed++;
    total++; if (test_default_frequency_fallback()) passed++;
    total++; if (test_invalid_non_integer_strings()) passed++;
    total++; if (test_out_of_bounds_port_ranges()) passed++;
    total++; if (test_zero_and_negative_map_dimensions()) passed++;
    total++; if (test_missing_team_names_after_flag()) passed++;
    printf("\n");

    // --- Memory Arena Suite ---
    printf("[SUITE] Memory Arenas\n");
    total++; if (test_arena_creation_and_free()) passed++;
    total++; if (test_arena_allocation_and_alignment()) passed++;
    total++; if (test_arena_out_of_memory()) passed++;
    total++; if (test_arena_reset()) passed++;

    printf("\n=========================================\n");
    printf("GLOBAL TEST RESULTS: %d/%d Passed\n", passed, total);
    printf("=========================================\n");

    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
