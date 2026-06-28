#include <criterion/criterion.h>
#include "logger/Logger.hpp"
#include "logger/ISink.hpp"
#include "logger/LogLevel.hpp"
#include <memory>
#include <string>
#include <vector>

using namespace zappy;

// --- mock sink ---

struct CaptureSink : ISink {
    std::vector<std::string> messages;
    void write(LogLevel, std::string_view msg) override {
        messages.emplace_back(msg);
    }
};

// --- LogLevel helpers ---

Test(LogLevel, to_string_trace)   { cr_assert(logLevelToString(LogLevel::TRACE)   == "TRACE"); }
Test(LogLevel, to_string_debug)   { cr_assert(logLevelToString(LogLevel::DEBUG)   == "DEBUG"); }
Test(LogLevel, to_string_info)    { cr_assert(logLevelToString(LogLevel::INFO)    == "INFO"); }
Test(LogLevel, to_string_warning) { cr_assert(logLevelToString(LogLevel::WARNING) == "WARN"); }
Test(LogLevel, to_string_error)   { cr_assert(logLevelToString(LogLevel::ERROR)   == "ERROR"); }
Test(LogLevel, to_string_none)    { cr_assert(logLevelToString(LogLevel::NONE)    == "NONE"); }

Test(LogLevel, from_string_trace)   { cr_assert_eq(stringToLogLevel("trace"),   LogLevel::TRACE); }
Test(LogLevel, from_string_debug)   { cr_assert_eq(stringToLogLevel("debug"),   LogLevel::DEBUG); }
Test(LogLevel, from_string_info)    { cr_assert_eq(stringToLogLevel("info"),    LogLevel::INFO); }
Test(LogLevel, from_string_warn)    { cr_assert_eq(stringToLogLevel("warn"),    LogLevel::WARNING); }
Test(LogLevel, from_string_warning) { cr_assert_eq(stringToLogLevel("warning"), LogLevel::WARNING); }
Test(LogLevel, from_string_error)   { cr_assert_eq(stringToLogLevel("error"),   LogLevel::ERROR); }
Test(LogLevel, from_string_none)    { cr_assert_eq(stringToLogLevel("none"),    LogLevel::NONE); }
Test(LogLevel, from_string_unknown) { cr_assert_eq(stringToLogLevel("garbage"), LogLevel::UNKNOWN); }

// --- Logger ---

Test(Logger, no_sinks_no_write) {
    Logger log;
    // just must not crash
    log.info("hello");
}

Test(Logger, sink_receives_message) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::TRACE);
    log.info("test message");
    cr_assert_eq(sink->messages.size(), 1u);
    cr_assert(sink->messages[0].find("test message") != std::string::npos);
}

Test(Logger, below_min_level_filtered) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::ERROR);
    log.debug("hidden");
    log.info("also hidden");
    cr_assert_eq(sink->messages.size(), 0u);
}

Test(Logger, at_min_level_passes) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::WARNING);
    log.warn("visible");
    cr_assert_eq(sink->messages.size(), 1u);
}

Test(Logger, above_min_level_passes) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::INFO);
    log.error("critical");
    cr_assert_eq(sink->messages.size(), 1u);
}

Test(Logger, multiple_sinks_both_receive) {
    auto s1 = std::make_shared<CaptureSink>();
    auto s2 = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(s1, LogLevel::TRACE);
    log.addSink(s2, LogLevel::TRACE);
    log.info("broadcast");
    cr_assert_eq(s1->messages.size(), 1u);
    cr_assert_eq(s2->messages.size(), 1u);
}

Test(Logger, set_min_level_filters_existing_sink) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::TRACE);
    log.setMinLevel(LogLevel::ERROR);
    log.info("filtered after setMinLevel");
    cr_assert_eq(sink->messages.size(), 0u);
}

Test(Logger, message_contains_origin) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::TRACE);
    log.log(LogLevel::INFO, "MyClass", "something happened");
    cr_assert(sink->messages[0].find("MyClass") != std::string::npos);
}

Test(Logger, message_without_origin_no_bracket) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::TRACE);
    log.info("no origin here");
    // no [ClassName] prefix when origin is empty
    cr_assert(sink->messages[0].find("no origin here") != std::string::npos);
}

Test(Logger, none_sink_level_never_writes) {
    auto sink = std::make_shared<CaptureSink>();
    Logger log;
    log.addSink(sink, LogLevel::NONE);
    log.error("should not appear");
    cr_assert_eq(sink->messages.size(), 0u);
}
