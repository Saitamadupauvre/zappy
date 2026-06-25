#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/InventoryProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include "world/WorldTypes.hpp"
#include <array>
#include <functional>
#include <memory>

namespace zappy {

class InventoryPanel {
public:
    static constexpr graphic::EntityID INVENTORY_HUD_ID = 9890;

    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader);

    void show(const Resources& inv);
    void hide();
    void toggle();
    bool isVisible() const;

    void setSelectedPlayer(uint32_t id);
    void onInventoryChanged(uint32_t changedId, const Resources& inv);
    void setSendLine(std::function<void(std::string)> fn);

private:
    std::function<void(std::string)> _sendLine;

    std::shared_ptr<InventoryProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior>  _container;
    std::array<graphic::TextureHandle, InventoryProvider::RESOURCE_COUNT> _textures{};
    uint32_t _selectedPlayerId = 0;
    ContextLogger _log{"InventoryPanel"};
};

} // namespace zappy
