#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/ResourceInfoProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include <array>
#include <memory>

namespace zappy {

class ResourceInfoPanel {
public:
    static constexpr graphic::EntityID RESOURCE_INFO_HUD_ID = 9999;

    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader);

    void show(int x, int y, int type);
    void hide();

private:
    std::shared_ptr<ResourceInfoProvider>            _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
    std::array<graphic::TextureHandle, 7>            _textures{};
    ContextLogger _log{"ResourceInfoPanel"};
};

} // namespace zappy
