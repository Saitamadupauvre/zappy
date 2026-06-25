#include "ClockPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"

namespace zappy {

void ClockPanel::setup(HudManager& hud)
{
    _provider = std::make_shared<ClockProvider>();

    auto entity = EntityBuilder(hud, CLOCK_HUD_ID, "clock_hud")
        .hud().container(_provider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 4.f)
        .hud().background(true, {10, 10, 30, 180}, {60, 80, 160, 200})
        .hud().anchor(graphic::Anchor::TopCenter)
        .hud().anchorOffset({0.f, 0.f})
        .hud().autoSize()
        .build();

    _container = entity->getBehavior<behavior::HudContainerBehavior>();
}

void ClockPanel::setUptime(unsigned long ticks)
{
    if (_provider) _provider->setUptime(ticks, _timeUnit);
}

void ClockPanel::setTimeUnit(int tu)
{
    _timeUnit = tu;
    if (_provider) _provider->setTimeUnit(tu);
}

void ClockPanel::tick(float dt)
{
    if (_provider) _provider->tick(dt);
}

} // namespace zappy
