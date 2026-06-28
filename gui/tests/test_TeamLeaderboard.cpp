#include <criterion/criterion.h>
#include "world/TeamLeaderboardStore.hpp"

using namespace zappy;

Test(TeamLeaderboardStore, add_team_tracked) {
    TeamLeaderboardStore store;
    store.addTeam("alpha");
    cr_assert(store.hasTeam("alpha"));
}

Test(TeamLeaderboardStore, unknown_team_not_present) {
    TeamLeaderboardStore store;
    cr_assert(!store.hasTeam("ghost"));
}

Test(TeamLeaderboardStore, player_added_to_team) {
    TeamLeaderboardStore store;
    store.addTeam("red");
    store.setPlayerTeam(1, "red");
    const auto& ranked = store.getRankedTeams();
    cr_assert_eq(ranked.size(), 1u);
    cr_assert_eq(ranked[0].playerCount, 1);
}

Test(TeamLeaderboardStore, player_removed_decrements_count) {
    TeamLeaderboardStore store;
    store.addTeam("blue");
    store.setPlayerTeam(1, "blue");
    store.setPlayerTeam(2, "blue");
    store.removePlayer(1);
    cr_assert_eq(store.getRankedTeams()[0].playerCount, 1);
}

Test(TeamLeaderboardStore, update_level_reflected_in_max) {
    TeamLeaderboardStore store;
    store.addTeam("t");
    store.setPlayerTeam(10, "t");
    store.updateLevel(10, 7);
    cr_assert_eq(store.getRankedTeams()[0].maxLevel, 7);
}

Test(TeamLeaderboardStore, ranked_sorted_by_max_level_desc) {
    TeamLeaderboardStore store;
    store.addTeam("low");
    store.addTeam("high");
    store.setPlayerTeam(1, "low");
    store.setPlayerTeam(2, "high");
    store.updateLevel(1, 2);
    store.updateLevel(2, 8);
    const auto& ranked = store.getRankedTeams();
    cr_assert_str_eq(ranked[0].name.c_str(), "high");
    cr_assert_str_eq(ranked[1].name.c_str(), "low");
}

Test(TeamLeaderboardStore, tied_max_level_sorted_alphabetically) {
    TeamLeaderboardStore store;
    store.addTeam("zebra");
    store.addTeam("apple");
    store.setPlayerTeam(1, "zebra");
    store.setPlayerTeam(2, "apple");
    store.updateLevel(1, 3);
    store.updateLevel(2, 3);
    const auto& ranked = store.getRankedTeams();
    cr_assert_str_eq(ranked[0].name.c_str(), "apple");
    cr_assert_str_eq(ranked[1].name.c_str(), "zebra");
}

Test(TeamLeaderboardStore, version_increments_on_change) {
    TeamLeaderboardStore store;
    uint64_t v0 = store.getVersion();
    store.addTeam("x");
    cr_assert(store.getVersion() > v0);
}

Test(TeamLeaderboardStore, get_players_for_team_sorted_by_level_desc) {
    TeamLeaderboardStore store;
    store.addTeam("t");
    store.setPlayerTeam(1, "t");
    store.setPlayerTeam(2, "t");
    store.updateLevel(1, 3);
    store.updateLevel(2, 7);
    const auto& players = store.getPlayersForTeam("t");
    cr_assert_eq(players.size(), 2u);
    cr_assert_eq(players[0].id, 2u);
    cr_assert_eq(players[0].level, 7);
}

Test(TeamLeaderboardStore, get_players_for_unknown_team_empty) {
    TeamLeaderboardStore store;
    cr_assert_eq(store.getPlayersForTeam("nobody").size(), 0u);
}

Test(TeamLeaderboardStore, default_player_level_is_one) {
    TeamLeaderboardStore store;
    store.addTeam("t");
    store.setPlayerTeam(5, "t");
    const auto& players = store.getPlayersForTeam("t");
    cr_assert_eq(players[0].level, 1);
}

Test(TeamLeaderboardStore, add_team_idempotent) {
    TeamLeaderboardStore store;
    uint64_t v0 = store.getVersion();
    store.addTeam("dup");
    store.addTeam("dup");
    cr_assert_eq(store.getRankedTeams().size(), 1u);
    cr_assert(store.getVersion() == v0 + 1);
}
