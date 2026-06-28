#include <criterion/criterion.h>
#include "parser/command/Command.hpp"

using namespace zappy;

Test(Command, callback_invoked_with_args) {
    bool called = false;
    std::vector<std::string> received;
    Command cmd("test", 1, [&](const std::vector<std::string>& args) {
        called = true;
        received = args;
    });
    cmd.execute({"hello", "world"});
    cr_assert(called);
    cr_assert_eq(received.size(), 2u);
    cr_assert_str_eq(received[0].c_str(), "hello");
}

Test(Command, too_few_args_callback_not_called) {
    bool called = false;
    Command cmd("test", 3, [&](const std::vector<std::string>&) { called = true; });
    cmd.execute({"a", "b"});
    cr_assert(!called);
}

Test(Command, exact_min_args_calls_callback) {
    bool called = false;
    Command cmd("test", 2, [&](const std::vector<std::string>&) { called = true; });
    cmd.execute({"x", "y"});
    cr_assert(called);
}

Test(Command, zero_min_args_always_calls) {
    bool called = false;
    Command cmd("test", 0, [&](const std::vector<std::string>&) { called = true; });
    cmd.execute({});
    cr_assert(called);
}

Test(Command, parse_arg_int) {
    int v = parseArg<int>("42");
    cr_assert_eq(v, 42);
}

Test(Command, parse_arg_string) {
    std::string v = parseArg<std::string>("hello");
    cr_assert_str_eq(v.c_str(), "hello");
}

Test(Command, parse_arg_invalid_int_throws) {
    cr_assert_throw(parseArg<int>("notanint"), std::runtime_error);
}

Test(Command, parse_arg_uint32) {
    uint32_t v = parseArg<uint32_t>("999");
    cr_assert_eq(v, 999u);
}
