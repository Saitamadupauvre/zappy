#include <criterion/criterion.h>
#include "cli/CliParser.hpp"

using namespace zappy;

static bool parse(CliParser& p, std::vector<const char*> argv) {
    return p.parseArguments((int)argv.size(), argv.data());
}

Test(CliParser, port_parsed) {
    CliParser p;
    bool ok = parse(p, {"gui", "-p", "4242"});
    cr_assert(ok);
    cr_assert_str_eq(p.getConfig().port.c_str(), "4242");
}

Test(CliParser, default_machine_localhost) {
    CliParser p;
    parse(p, {"gui", "-p", "1234"});
    cr_assert_str_eq(p.getConfig().machine.c_str(), "localhost");
}

Test(CliParser, host_overridden) {
    CliParser p;
    parse(p, {"gui", "-p", "1234", "-h", "myserver"});
    cr_assert_str_eq(p.getConfig().machine.c_str(), "myserver");
}

Test(CliParser, verbose_level_debug) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "debug"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::DEBUG);
}

Test(CliParser, verbose_level_info) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "info"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::INFO);
}

Test(CliParser, verbose_level_trace) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "trace"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::TRACE);
}

Test(CliParser, verbose_level_none) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "none"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::NONE);
}

Test(CliParser, verbose_level_error) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "error"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::ERROR);
}

Test(CliParser, verbose_level_warn) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "-v", "warn"});
    cr_assert_eq(p.getConfig().consoleLog.level, LogLevel::WARNING);
}

Test(CliParser, invalid_verbose_level_throws) {
    CliParser p;
    cr_assert_throw(parse(p, {"gui", "-p", "1", "-v", "supersonic"}), CliParserException);
}

Test(CliParser, log_file_sets_path) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "--log-file", "out.log"});
    cr_assert_str_eq(p.getConfig().fileLog.filePath.c_str(), "out.log");
}

Test(CliParser, log_file_sets_default_trace_level) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "--log-file", "out.log"});
    cr_assert_eq(p.getConfig().fileLog.level, LogLevel::TRACE);
}

Test(CliParser, log_file_level_override) {
    CliParser p;
    parse(p, {"gui", "-p", "1", "--log-file", "out.log", "--log-file-level", "warn"});
    cr_assert_eq(p.getConfig().fileLog.level, LogLevel::WARNING);
}

Test(CliParser, invalid_log_file_level_throws) {
    CliParser p;
    cr_assert_throw(parse(p, {"gui", "-p", "1", "--log-file", "out.log", "--log-file-level", "bad"}), CliParserException);
}

Test(CliParser, unknown_arg_throws) {
    CliParser p;
    cr_assert_throw(parse(p, {"gui", "--unknown"}), CliParserException);
}

Test(CliParser, help_flag_returns_false) {
    CliParser p;
    bool ok = parse(p, {"gui", "--help"});
    cr_assert(!ok);
}

Test(CliParser, log_file_flag_with_dash_value_throws) {
    CliParser p;
    cr_assert_throw(parse(p, {"gui", "-p", "1", "--log-file", "-bad"}), CliParserException);
}

Test(CliParser, value_flag_missing_value_throws) {
    CliParser p;
    cr_assert_throw(parse(p, {"gui", "-p"}), CliParserException);
}
