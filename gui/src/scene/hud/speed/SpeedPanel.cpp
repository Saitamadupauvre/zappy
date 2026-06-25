#include "SpeedPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void SpeedPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<SpeedControlProvider>();
    _provider->setOnCommit([this](int tu) {
        if (_sendLine) _sendLine("sst " + std::to_string(tu));
    });

    auto entity = EntityBuilder(hud, SPEED_HUD_ID, "speed_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 6.f)
        .hud().background(true, {10, 10, 20, 200}, {60, 60, 120, 200})
        .hud().anchor(graphic::Anchor::BottomLeft)
        .hud().anchorOffset({0.f, 0.f})
        .hud().autoSize()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void SpeedPanel::setSendLine(std::function<void(std::string)> fn)
{
    _sendLine = std::move(fn);
}

void SpeedPanel::setTimeUnit(int tu)
{
    if (_provider) _provider->setTimeUnit(tu);
}

} // namespace zappy
