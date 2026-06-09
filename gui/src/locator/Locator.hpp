#pragma once
#include "logger/Logger.hpp"

namespace zappy {

class Locator
{
    public:
        static void provide(Logger* logger);
        static Logger* getLogger();

    private:
        static Logger* _logger;
};

} // namespace zappy