#include "ResourceInfoPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void ResourceInfoPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                               graphic::ITextureLoader& loader)
{
    _provider = std::make_shared<ResourceInfoProvider>();

    static constexpr const char* RESOURCE_PATHS[7] = {
        "assets/images/food/apple.jpg",
        "assets/images/ores/linemate.jpg",
        "assets/images/ores/deraumere.jpg",
        "assets/images/ores/sibur.jpg",
        "assets/images/ores/mendiane.jpg",
        "assets/images/ores/phiras.jpg",
        "assets/images/ores/thystame.png"
    };
    for (int i = 0; i < 7; ++i) {
        try {
            auto texData = loader.loadFromFile(RESOURCE_PATHS[i]);
            _textures[i] = renderer.uploadTexture(texData);
        } catch (...) {
            _log.warn("Failed to load resource info texture ", i);
        }
    }
    _provider->setResourceTextures(_textures);

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
