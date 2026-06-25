
#include "config.h"
#include "arena.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

extern bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg);

#define ASSERT_TEST(cond, message) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s\n", message); \
            return false; \
        } \
    } while (0)

// Global backing arena used exclusively across this test file session
static arena_t test_suite_arena;
static bool arena_initialized = false;

// Helper function to wipe primitive parameters while maintaining our safe arena backing pool
void reset_test_env(prog_cfg_t *cfg) {
    // 1. If our global backing arena hasn't been allocated yet, spin it up now (1MB)
    if (!arena_initialized) {
        test_suite_arena = create_mem_arena(1024 * 1024);
        arena_initialized = true;
    }

    // 2. Reset the tracking utilization threshold of our arena block back to 0
    reset_mem_arena(&test_suite_arena);

    if (cfg != NULL) {
        // 3. Clear out everything inside the cfg struct space to absolute 0
        memset(cfg, 0, sizeof(prog_cfg_t));
        
        // 4. Bind our live, healthy backing arena right into the structure
        cfg->perm_mem_arena = test_suite_arena;
    }

    // 5. Hard reset glibc's getopt internal cache registers to index 0
    optind = 0;
    opterr = 0;
    optopt = 0;
}

// ============================================================================
// TEST CASES
// ============================================================================

bool test_valid_standard_args(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg); // Prepares clean fields and binds our active 1MB arena

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "20", "-c", "2", "-n", "TeamA", "TeamB", "-f", "50"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);

    ASSERT_TEST(result == true, "Parser rejected completely valid standard flags.");
    ASSERT_TEST(cfg.port == 4242, "Port was parsed incorrectly.");
    ASSERT_TEST(cfg.w == 10, "Width was parsed incorrectly.");
    ASSERT_TEST(cfg.h == 20, "Height was parsed incorrectly.");
    ASSERT_TEST(cfg.client_count == 2, "Client count was parsed incorrectly.");
    ASSERT_TEST(cfg.frequency == 50, "Frequency was parsed incorrectly.");
    ASSERT_TEST(cfg.team_count == 2, "Team count was calculated incorrectly.");
    ASSERT_TEST(strcmp(cfg.team_names[0], "TeamA") == 0, "First team name extraction mismatch.");
    ASSERT_TEST(strcmp(cfg.team_names[1], "TeamB") == 0, "Second team name extraction mismatch.");
    ASSERT_TEST(cfg.team_names[2] == NULL, "Team names array is missing its NULL termination.");

    return true;
}

bool test_missing_mandatory_args(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "20", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser incorrectly allowed execution with missing mandatory flags.");

    return true;
}

bool test_default_frequency_fallback(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "10", "-c", "5", "-n", "SoloTeam"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == true, "Parser rejected arguments missing the optional frequency flag.");
    ASSERT_TEST(cfg.frequency == 100, "Parser failed to default frequency to 100 as per Zappy specs.");

    return true;
}

bool test_invalid_non_integer_strings(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10abc", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser accepted alphanumeric junk string for an integer field.");

    return true;
}

bool test_out_of_bounds_port_ranges(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "99999", "-x", "10", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser accepted an invalid port number exceeding limits.");

    return true;
}

bool test_zero_and_negative_map_dimensions(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "0", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser incorrectly validated a width configuration value of 0.");

    return true;
}

bool test_missing_team_names_after_flag(void) {
    prog_cfg_t cfg;
    reset_test_env(&cfg);

    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "10", "-c", "2", "-n", "-f", "100"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser allowed empty team definitions when followed directly by a flag.");

    return true;
}
