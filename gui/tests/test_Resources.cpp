#include <criterion/criterion.h>
#include <stdexcept>
#include "parser/Resources/Resources.hpp"

using namespace zappy;

Test(Resources, parse_seven_values) {
    std::vector<std::string> args = {"5", "1", "2", "3", "4", "6", "7"};
    size_t idx = 0;
    Resources r = parseArg<Resources>(args, idx);
    cr_assert_eq(r.food,      5);
    cr_assert_eq(r.linemate,  1);
    cr_assert_eq(r.deraumere, 2);
    cr_assert_eq(r.sibur,     3);
    cr_assert_eq(r.mendiane,  4);
    cr_assert_eq(r.phiras,    6);
    cr_assert_eq(r.thystame,  7);
    cr_assert_eq(idx, 7u);
}

Test(Resources, parse_advances_index_from_offset) {
    std::vector<std::string> args = {"X", "Y", "10", "2", "3", "4", "5", "6", "7"};
    size_t idx = 2;
    Resources r = parseArg<Resources>(args, idx);
    cr_assert_eq(r.food, 10);
    cr_assert_eq(idx, 9u);
}

Test(Resources, parse_not_enough_args_throws) {
    std::vector<std::string> args = {"1", "2", "3"};
    size_t idx = 0;
    cr_assert_throw(parseArg<Resources>(args, idx), std::runtime_error);
}

Test(Resources, default_values_zero) {
    Resources r;
    cr_assert_eq(r.food, 0);
    cr_assert_eq(r.linemate, 0);
    cr_assert_eq(r.deraumere, 0);
    cr_assert_eq(r.sibur, 0);
    cr_assert_eq(r.mendiane, 0);
    cr_assert_eq(r.phiras, 0);
    cr_assert_eq(r.thystame, 0);
}

Test(Resources, parse_zeros) {
    std::vector<std::string> args = {"0", "0", "0", "0", "0", "0", "0"};
    size_t idx = 0;
    Resources r = parseArg<Resources>(args, idx);
    cr_assert_eq(r.food, 0);
    cr_assert_eq(r.thystame, 0);
}
