#include <criterion/criterion.h>
#include "world/TeamChatStore.hpp"

using namespace zappy;

Test(TeamChatStore, message_from_unknown_player_ignored) {
    TeamChatStore store;
    store.addMessage(42, "hello");
    cr_assert_eq(store.getTeamMessages("alpha").size(), 0u);
}

Test(TeamChatStore, message_stored_after_team_set) {
    TeamChatStore store;
    store.setPlayerTeam(1, "alpha");
    store.addMessage(1, "hi");
    const auto& msgs = store.getTeamMessages("alpha");
    cr_assert_eq(msgs.size(), 1u);
    cr_assert_eq(msgs[0].playerId, 1u);
    cr_assert_str_eq(msgs[0].text.c_str(), "hi");
}

Test(TeamChatStore, multiple_players_same_team) {
    TeamChatStore store;
    store.setPlayerTeam(1, "red");
    store.setPlayerTeam(2, "red");
    store.addMessage(1, "a");
    store.addMessage(2, "b");
    cr_assert_eq(store.getTeamMessages("red").size(), 2u);
}

Test(TeamChatStore, players_different_teams_isolated) {
    TeamChatStore store;
    store.setPlayerTeam(1, "red");
    store.setPlayerTeam(2, "blue");
    store.addMessage(1, "red-msg");
    store.addMessage(2, "blue-msg");
    cr_assert_eq(store.getTeamMessages("red").size(), 1u);
    cr_assert_eq(store.getTeamMessages("blue").size(), 1u);
}

Test(TeamChatStore, get_team_for_player) {
    TeamChatStore store;
    store.setPlayerTeam(5, "green");
    cr_assert_str_eq(store.getTeamForPlayer(5).c_str(), "green");
}

Test(TeamChatStore, get_team_for_unknown_player_empty) {
    TeamChatStore store;
    cr_assert_str_eq(store.getTeamForPlayer(999).c_str(), "");
}

Test(TeamChatStore, remove_player_stops_messages) {
    TeamChatStore store;
    store.setPlayerTeam(3, "alpha");
    store.removePlayer(3);
    store.addMessage(3, "should not appear");
    cr_assert_eq(store.getTeamMessages("alpha").size(), 0u);
}

Test(TeamChatStore, version_increments_on_message) {
    TeamChatStore store;
    store.setPlayerTeam(1, "t");
    uint64_t v0 = store.getVersion();
    store.addMessage(1, "x");
    cr_assert(store.getVersion() > v0);
}

Test(TeamChatStore, unknown_team_returns_empty_vec) {
    TeamChatStore store;
    cr_assert_eq(store.getTeamMessages("nosuchteam").size(), 0u);
}

Test(TeamChatStore, messages_capped_at_max) {
    TeamChatStore store;
    store.setPlayerTeam(1, "t");
    for (int i = 0; i < 205; ++i)
        store.addMessage(1, "msg");
    cr_assert(store.getTeamMessages("t").size() <= 200u);
}
