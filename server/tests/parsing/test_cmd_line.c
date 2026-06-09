#include "config.h" // Ensures prog_cfg_t, PORT_MIN, PORT_MAX are visible

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // Handles all getopt state variables natively

// External declaration of your parsing function
extern bool parse_cmd_line(int argc, char *argv[], prog_cfg_t *prog_cfg);

// Lightweight assert macro for testing
#define ASSERT_TEST(cond, message) \
    do { \
        if (!(cond)) { \
            printf("  [FAIL] %s\n", message); \
            return false; \
        } \
    } while (0)

// Helper function to wipe out and reset configuration structures between test runs
void reset_test_env(prog_cfg_t *cfg) {
    if (cfg != NULL) {
        // Since parsing.c deep-copies team names via strdup, we safely free each entry
        if (cfg->team_names != NULL) {
            for (int i = 0; cfg->team_names[i] != NULL; i++) {
                free(cfg->team_names[i]); // Free individual deep-copied strings
                cfg->team_names[i] = NULL;
            }
            free(cfg->team_names);        // Free the master pointer array
            cfg->team_names = NULL;
        }
    }
    // Fully reset getopt variables back to deterministic defaults
    optind = 1;
    opterr = 0;
    optopt = 0;
}

// ============================================================================
// TEST CASES
// ============================================================================

bool test_valid_standard_args(void) {
    prog_cfg_t cfg = {0}; // Explicitly zeroed
    optind = 1;

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

    reset_test_env(&cfg);
    return true;
}

bool test_missing_mandatory_args(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed to prevent garbage pointer reads
    optind = 1;

    // Missing the mandatory "-c" (client count) flag completely
    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "20", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser incorrectly allowed execution with missing mandatory flags.");

    reset_test_env(&cfg);
    return true;
}

bool test_default_frequency_fallback(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed
    optind = 1;

    // Valid command line arguments but omitting the optional "-f" flag entirely
    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "10", "-c", "5", "-n", "SoloTeam"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == true, "Parser rejected arguments missing the optional frequency flag.");
    ASSERT_TEST(cfg.frequency == 100, "Parser failed to default frequency to 100 as per Zappy specs.");

    reset_test_env(&cfg);
    return true;
}

bool test_invalid_non_integer_strings(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed
    optind = 1;

    // Passing alphanumeric text "10abc" to the width parsing module
    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10abc", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser accepted alphanumeric junk string for an integer field.");

    reset_test_env(&cfg);
    return true;
}

bool test_out_of_bounds_port_ranges(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed
    optind = 1;

    // Testing maximum out of bounds port boundaries (> 65535)
    char *argv[] = {"./zappy_server", "-p", "99999", "-x", "10", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser accepted an invalid port number exceeding limits.");

    reset_test_env(&cfg);
    return true;
}

bool test_zero_and_negative_map_dimensions(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed
    optind = 1;

    // Width set to 0, which is structurally invalid for map operations
    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "0", "-y", "10", "-c", "2", "-n", "Team1"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser incorrectly validated a width configuration value of 0.");

    reset_test_env(&cfg);
    return true;
}

bool test_missing_team_names_after_flag(void) {
    prog_cfg_t cfg = {0}; // Fixed: Explicitly zeroed
    optind = 1;

    // Putting another flag immediately after "-n", leaving team names empty
    char *argv[] = {"./zappy_server", "-p", "4242", "-x", "10", "-y", "10", "-c", "2", "-n", "-f", "100"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    bool result = parse_cmd_line(argc, argv, &cfg);
    ASSERT_TEST(result == false, "Parser allowed empty team definitions when followed directly by a flag.");

    reset_test_env(&cfg);
    return true;
}

// ============================================================================
// UNIT TEST RUNNER
// ============================================================================

int main(void) {
    int passed = 0;
    int total = 7;

    printf("Starting Zappy Configuration Unit Tests...\n\n");

    if (test_valid_standard_args()) passed++;
    if (test_missing_mandatory_args()) passed++;
    if (test_default_frequency_fallback()) passed++;
    if (test_invalid_non_integer_strings()) passed++;
    if (test_out_of_bounds_port_ranges()) passed++;
    if (test_zero_and_negative_map_dimensions()) passed++;
    if (test_missing_team_names_after_flag()) passed++;

    printf("\n=========================================\n");
    printf("TEST RESULTS: %d/%d Passed\n", passed, total);
    printf("=========================================\n");

    return (passed == total) ? EXIT_SUCCESS : EXIT_FAILURE;
}
