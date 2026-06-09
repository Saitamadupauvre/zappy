#pragma once
#include "ISink.hpp"
#include <iostream>

namespace zappy {

class ConsoleSink : public ISink
{
    public:
        ConsoleSink() = default;
        ~ConsoleSink() = default;

        void write(LogLevel level, std::string_view formattedMessage) override {
            if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
                std::cerr << formattedMessage << std::flush;
            } else {
                std::cout << formattedMessage << std::flush;
            }
        }
};

} // namespace zappy