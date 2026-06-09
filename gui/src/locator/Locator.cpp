#include "Locator.hpp"

namespace zappy {

Logger* Locator::_logger = nullptr;

void Locator::provide(Logger* logger) {
    _logger = logger;
}

Logger* Locator::getLogger() {
    return _logger;
}

} // namespace zappy