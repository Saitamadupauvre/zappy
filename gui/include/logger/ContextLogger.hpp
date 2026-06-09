#pragma once
#include "locator/Locator.hpp"
#include <string_view>
#include <string>

namespace zappy {

class ContextLogger
{
    public:
        explicit ContextLogger(std::string_view className) : _className(className) {}
        ~ContextLogger() = default;

        template <typename... Args>
        void trace(Args&&... args) const { if (Locator::getLogger()) Locator::getLogger()->traceWithContext(_className, std::forward<Args>(args)...); }

        template <typename... Args>
        void debug(Args&&... args) const { if (Locator::getLogger()) Locator::getLogger()->debugWithContext(_className, std::forward<Args>(args)...); }

        template <typename... Args>
        void info(Args&&... args) const { if (Locator::getLogger()) Locator::getLogger()->infoWithContext(_className, std::forward<Args>(args)...); }

        template <typename... Args>
        void warn(Args&&... args) const { if (Locator::getLogger()) Locator::getLogger()->warnWithContext(_className, std::forward<Args>(args)...); }

        template <typename... Args>
        void error(Args&&... args) const { if (Locator::getLogger()) Locator::getLogger()->errorWithContext(_className, std::forward<Args>(args)...); }

    private:
        std::string _className;
};

} // namespace zappy