#include "ResourceInfoPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void ResourceInfoPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                               graphic::ITextureLoader& loader)
{
    _provider = std::make_shared<ResourceInfoProvider>();

    try {
        auto texData = loader.loadFromFile("assets/images/pikmin.jpg");
        auto handle  = renderer.uploadTexture(texData);
        _provider->setImage(handle, 64.0f, 64.0f);
    } catch (...) {
        _log.warn("Failed to load resource info texture");
    }

    auto entity = EntityBuilder(hud, RESOURCE_INFO_HUD_ID, "hud_resource_info")
        .hud().container(_provider)
        .hud().infoHud(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 8.0f)
        .hud().background(true, {10, 10, 20, 190}, {70, 70, 120, 220})
        .hud().anchor(graphic::Anchor::TopRight)
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void ResourceInfoPanel::show(int x, int y, int type)
{
    if (_provider) _provider->updateData(x, y, type);
    if (_container) _container->setVisible(true);
}

void ResourceInfoPanel::hide()
{
    if (_provider) _provider->clear();
    if (_container) _container->setVisible(false);
}

} // namespace zappy
