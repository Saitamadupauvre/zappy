#include <criterion/criterion.h>
#include "world/World.hpp"
#include "event/WorldEvent.hpp"

using namespace zappy;

// --- resize ---

Test(World, resize_sets_dimensions) {
    World w;
    w.resize(10, 5);
    cr_assert_eq(w.getWidth(), 10);
    cr_assert_eq(w.getHeight(), 5);
    cr_assert(w.isReady());
}

Test(World, resize_emits_event) {
    World w;
    bool got = false;
    w.setEventDispatcher([&](const event::WorldEvent& ev) {
        std::visit([&](auto& e) {
            if constexpr (std::is_same_v<std::decay_t<decltype(e)>, event::WorldResizedEvent>) {
                cr_assert_eq(e.width, 4);
                cr_assert_eq(e.height, 3);
                got = true;
            }
        }, ev);
    });
    w.resize(4, 3);
    cr_assert(got);
}

Test(World, default_not_ready) {
    World w;
    cr_assert(!w.isReady());
}

// --- tiles ---

Test(World, set_tile_updates_resources) {
    World w;
    w.resize(5, 5);
    Resources res;
    res.food = 3; res.linemate = 1;
    w.setTile(2, 2, res);
    const Tile& t = w.getTile(2, 2);
    cr_assert_eq(t.resources.food, 3);
    cr_assert_eq(t.resources.linemate, 1);
}

Test(World, set_tile_out_of_bounds_ignored) {
    World w;
    w.resize(3, 3);
    Resources res; res.food = 99;
    w.setTile(10, 10, res);  // out-of-bounds, should not crash
    cr_assert_eq(w.getTile(10, 10).resources.food, 0);  // returns empty tile
}

// --- players ---

Test(World, add_player_stored) {
    World w;
    w.resize(10, 10);
    PlayerState p;
    p.id = 1; p.x = 3; p.y = 4; p.level = 2; p.team = "alpha";
    w.addPlayer(p);
    const PlayerState* found = w.getPlayer(1);
    cr_assert_not_null(found);
    cr_assert_eq(found->x, 3);
    cr_assert_eq(found->level, 2);
}

Test(World, get_player_missing_returns_null) {
    World w;
    cr_assert_null(w.getPlayer(999));
}

Test(World, move_player_updates_position) {
    World w;
    w.resize(10, 10);
    PlayerState p; p.id = 7; p.x = 0; p.y = 0;
    w.addPlayer(p);
    w.movePlayer(7, 5, 6, 2);
    const PlayerState* found = w.getPlayer(7);
    cr_assert_not_null(found);
    cr_assert_eq(found->x, 5);
    cr_assert_eq(found->y, 6);
    cr_assert_eq(found->orientation, 2);
}

Test(World, move_unknown_player_no_crash) {
    World w;
    w.movePlayer(404, 1, 1, 1);  // no crash
}

Test(World, remove_player_erases) {
    World w;
    w.resize(5, 5);
    PlayerState p; p.id = 42;
    w.addPlayer(p);
    w.removePlayer(42);
    cr_assert_null(w.getPlayer(42));
}

Test(World, set_player_level) {
    World w;
    w.resize(5, 5);
    PlayerState p; p.id = 3; p.level = 1;
    w.addPlayer(p);
    w.setPlayerLevel(3, 5);
    cr_assert_eq(w.getPlayer(3)->level, 5);
}

Test(World, set_player_inventory) {
    World w;
    w.resize(5, 5);
    PlayerState p; p.id = 2;
    w.addPlayer(p);
    Resources inv; inv.food = 10; inv.phiras = 3;
    w.setPlayerInventory(2, inv);
    cr_assert_eq(w.getPlayer(2)->inventory.food, 10);
    cr_assert_eq(w.getPlayer(2)->inventory.phiras, 3);
}

// --- eggs ---

Test(World, add_and_remove_egg) {
    World w;
    EggState egg; egg.id = 100; egg.playerId = 1; egg.x = 2; egg.y = 3; egg.team = "beta";
    w.addEgg(egg);
    cr_assert_not_null(w.getEgg(100));
    cr_assert_eq(w.getEgg(100)->x, 2);
    w.removeEgg(100);
    cr_assert_null(w.getEgg(100));
}

// --- teams ---

Test(World, add_team_stored) {
    World w;
    w.addTeam("red");
    w.addTeam("blue");
    const auto& teams = w.getTeams();
    cr_assert_eq(teams.size(), 2u);
    cr_assert_str_eq(teams[0].c_str(), "red");
    cr_assert_str_eq(teams[1].c_str(), "blue");
}

// --- time unit ---

Test(World, set_time_unit) {
    World w;
    w.setTimeUnit(42);
    cr_assert_eq(w.getTimeUnit(), 42);
}

Test(World, set_time_unit_zero_clamps_to_one) {
    World w;
    w.setTimeUnit(0);
    cr_assert_eq(w.getTimeUnit(), 1);
}
