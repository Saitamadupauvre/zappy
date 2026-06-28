#include <criterion/criterion.h>
#include "parser/CommandParser/CommandParser.hpp"
#include "network/GuiProtocol.hpp"

using namespace zappy;
using Kind = zappy::net::MessageKind;

Test(CommandParser, known_command_msz) {
    CommandParser p;
    auto msg = p.parseLine("msz 10 20");
    cr_assert_eq(msg.kind, Kind::Msz);
    cr_assert_eq(msg.args.size(), 2u);
    cr_assert_str_eq(msg.args[0].c_str(), "10");
}

Test(CommandParser, known_command_pnw) {
    CommandParser p;
    auto msg = p.parseLine("pnw 1 3 4 1 1 alpha");
    cr_assert_eq(msg.kind, Kind::Pnw);
    cr_assert_eq(msg.args.size(), 6u);
}

Test(CommandParser, known_command_bct) {
    CommandParser p;
    auto msg = p.parseLine("bct 0 0 1 0 0 0 0 0 0");
    cr_assert_eq(msg.kind, Kind::Bct);
}

Test(CommandParser, known_command_tna) {
    CommandParser p;
    auto msg = p.parseLine("tna red");
    cr_assert_eq(msg.kind, Kind::Tna);
    cr_assert_str_eq(msg.args[0].c_str(), "red");
}

Test(CommandParser, known_command_ppo) {
    CommandParser p;
    auto msg = p.parseLine("ppo 5 2 3 1");
    cr_assert_eq(msg.kind, Kind::Ppo);
}

Test(CommandParser, known_command_plv) {
    CommandParser p;
    auto msg = p.parseLine("plv 1 3");
    cr_assert_eq(msg.kind, Kind::Plv);
}

Test(CommandParser, known_command_pin) {
    CommandParser p;
    auto msg = p.parseLine("pin 1 0 0 5 0 0 0 0 0 0");
    cr_assert_eq(msg.kind, Kind::Pin);
}

Test(CommandParser, known_command_pex) {
    CommandParser p;
    auto msg = p.parseLine("pex 2");
    cr_assert_eq(msg.kind, Kind::Pex);
}

Test(CommandParser, known_command_pbc) {
    CommandParser p;
    auto msg = p.parseLine("pbc 3 hello world");
    cr_assert_eq(msg.kind, Kind::Pbc);
    cr_assert_eq(msg.args.size(), 3u);
}

Test(CommandParser, known_command_seg) {
    CommandParser p;
    auto msg = p.parseLine("seg blue");
    cr_assert_eq(msg.kind, Kind::Seg);
}

Test(CommandParser, known_command_sgt) {
    CommandParser p;
    auto msg = p.parseLine("sgt 100");
    cr_assert_eq(msg.kind, Kind::Sgt);
}

Test(CommandParser, known_command_sst) {
    CommandParser p;
    auto msg = p.parseLine("sst 50");
    cr_assert_eq(msg.kind, Kind::Sst);
}

Test(CommandParser, bare_number_maps_to_stu) {
    CommandParser p;
    auto msg = p.parseLine("12345");
    cr_assert_eq(msg.kind, Kind::Stu);
    cr_assert_str_eq(msg.args[0].c_str(), "12345");
}

Test(CommandParser, unknown_command) {
    CommandParser p;
    auto msg = p.parseLine("garbage 1 2 3");
    cr_assert_eq(msg.kind, Kind::Unknown);
}

Test(CommandParser, empty_line_returns_unknown) {
    CommandParser p;
    auto msg = p.parseLine("");
    cr_assert_eq(msg.kind, Kind::Unknown);
}

Test(CommandParser, raw_field_preserved) {
    CommandParser p;
    auto msg = p.parseLine("msz 4 4");
    cr_assert_str_eq(msg.raw.c_str(), "msz 4 4");
}

Test(CommandParser, enw_command) {
    CommandParser p;
    auto msg = p.parseLine("enw 10 5 3 2 alpha");
    cr_assert_eq(msg.kind, Kind::Enw);
}

Test(CommandParser, ebo_command) {
    CommandParser p;
    auto msg = p.parseLine("ebo 10");
    cr_assert_eq(msg.kind, Kind::Ebo);
}

Test(CommandParser, edi_command) {
    CommandParser p;
    auto msg = p.parseLine("edi 10");
    cr_assert_eq(msg.kind, Kind::Edi);
}
