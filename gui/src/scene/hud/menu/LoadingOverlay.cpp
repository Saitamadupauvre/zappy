#include "LoadingOverlay.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

const std::vector<behavior::hud::HudElement>& LoadingProvider::getHudElements() const
{
    if (!_dirty) return _cache;
    _dirty = false;
    _cache.clear();

    _cache.push_back({behavior::hud::TextData{_text, 18.f, {200, 220, 255, 255}}});

    _cache.push_back({behavior::hud::BarData{
        _ratio,
        {40, 110, 210, 220},
        260.f,
        12.f
    }});
    return _cache;
}

void LoadingOverlay::setup(HudManager& hud)
{
    _provider = std::make_shared<LoadingProvider>();

    auto entity = EntityBuilder(hud, ID, "loading_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 16.f)
        .hud().anchor(graphic::Anchor::Center)
        .hud().background(true, {5, 5, 15, 230}, {30, 40, 100, 220})
        .hud().boxSize({300.f, 100.f})
        .hud().hidden()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void LoadingOverlay::setProgress(float ratio) { if (_provider) _provider->setProgress(ratio); }
void LoadingOverlay::setText(const std::string& t) { if (_provider) _provider->setText(t); }
void LoadingOverlay::show() { if (_container) _container->setVisible(true); }
void LoadingOverlay::hide() { if (_container) _container->setVisible(false); }
bool LoadingOverlay::isVisible() const { return _container && _container->isVisible(); }

} // namespace zappy
