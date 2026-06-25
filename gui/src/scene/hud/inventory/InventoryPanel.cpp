#include "InventoryPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void InventoryPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                            graphic::ITextureLoader& loader)
{
    _provider = std::make_shared<InventoryProvider>();

    for (int i = 0; i < InventoryProvider::RESOURCE_COUNT; ++i) {
        try {
            auto texData = loader.loadFromFile("assets/images/pikmin.jpg");
            _textures[i] = renderer.uploadTexture(texData);
        } catch (...) {
            _log.warn("Failed to load resource texture ", i);
        }
    }
    _provider->setResourceTextures(_textures);

    auto entity = EntityBuilder(hud, INVENTORY_HUD_ID, "inventory_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Grid, 6.f, 3)
        .hud().background(true, {12, 12, 18, 220}, {60, 60, 100, 220})
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().anchorOffset({-190.f, 0.f})
        .hud().boxSize({196.f, 210.f})
        .hud().title("Inventory", 13.f)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void InventoryPanel::show(const Resources& inv)
{
    if (_provider) _provider->setInventory(inv);
    if (_container) _container->setVisible(true);
}

void InventoryPanel::hide()
{
    if (_container) _container->setVisible(false);
}

bool InventoryPanel::isVisible() const
{
    return _container && _container->isFullyVisible();
}

void InventoryPanel::setSendLine(std::function<void(std::string)> fn)
{
    _sendLine = std::move(fn);
}

void InventoryPanel::setSelectedPlayer(uint32_t id)
{
    _selectedPlayerId = id;
    if (_provider) _provider->setPlayer(id);
    hide();
}

void InventoryPanel::toggle()
{
    if (!_container) return;
    if (_container->isFullyVisible()) {
        _container->setVisible(false);
    } else {
        if (_sendLine && _selectedPlayerId != 0)
            _sendLine("pin #" + std::to_string(_selectedPlayerId));
        _container->setVisible(true);
    }
}

void InventoryPanel::onInventoryChanged(uint32_t changedId, const Resources& inv)
{
    if (changedId != _selectedPlayerId) return;
    if (_provider) _provider->setInventory(inv);
}

} // namespace zappy
