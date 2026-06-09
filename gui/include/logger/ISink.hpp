#pragma once
#include "LogLevel.hpp"
#include <string_view>

namespace zappy {

class ISink
{
    public:
        virtual ~ISink() = default;
        virtual void write(LogLevel level, std::string_view formattedMessage) = 0;
};

} // namespace zappy