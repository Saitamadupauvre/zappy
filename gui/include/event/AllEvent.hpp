#pragma once

#include "graphic/Types.hpp"
#include "event/WorldEvent.hpp"
#include "event/WindowEvent.hpp"
#include "event/RenderEvent.hpp"
#include "event/LogicEvent.hpp"

namespace event {

using Event = std::variant<WindowEvent, RenderEvent, WorldEvent, LogicEvent>;

} // namespace event
