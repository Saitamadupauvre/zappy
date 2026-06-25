#pragma once

#include "graphic/Types.hpp"

namespace graphic { class IRenderer; }

namespace event {

struct RenderEvent { graphic::IRenderer& renderer; graphic::Vector2f viewportSize; };

} // namespace event
