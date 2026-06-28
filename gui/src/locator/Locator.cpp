#include "locator/Locator.hpp"

namespace zappy {

Logger* Locator::_logger = nullptr;
IScene* Locator::_scene  = nullptr;
graphic::IRenderer* Locator::_renderer = nullptr;
graphic::FontHandle Locator::_defaultFont = {};
graphic::FontHandle Locator::_cjkFont     = {};

} // namespace zappy
