#include "PopupPanel.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include <algorithm>
#include <limits>

namespace zappy {

void PopupPanel::setup(HudManager& hud, graphic::IRenderer& renderer,
                       graphic::ITextureLoader& loader)
{
    graphic::TextureHandle popupTex{};
    try {
        auto texData = loader.loadFromFile("assets/images/pikmin.jpg");
        popupTex     = renderer.uploadTexture(texData);
    } catch (...) {
        _log.warn("Failed to load popup texture");
    }

    for (int i = 0; i < MAX_POPUP_SLOTS; ++i) {
        graphic::EntityID id = POPUP_BASE_ID + static_cast<graphic::EntityID>(i);

        auto provider = std::make_shared<NotificationPopupProvider>();
        if (popupTex.id != 0)
            provider->setTexture(popupTex, 56.f, 56.f);
        _slots[i].provider = provider;

        auto entity = EntityBuilder(hud, id, "popup_hud")
            .hud().container(provider)
            .hud().layout(behavior::hud::LayoutEngine::Type::MediaObject, 8.0f)
            .hud().background(true, {10, 10, 20, 210}, {70, 70, 120, 220})
            .hud().anchor(graphic::Anchor::BottomRight)
            .hud().anchorOffset({0.0f, 0.0f})
            .hud().boxSize({300.0f, 80.0f})
            .hud().hidden()
            .build();

        _slots[i].container = entity->getBehavior<behavior::HudContainerBehavior>();
    }
}

void PopupPanel::push(const std::string& title, const std::string& subtitle,
                      graphic::Color4b color)
{
    int activeCount = 0;
    for (auto& s : _slots)
        if (s.timer > 0.f) ++activeCount;

    float dur = std::max(1.2f, POPUP_DURATION / std::max(1, activeCount));

    int   freeIdx = -1, oldestIdx = 0;
    float minTimer = std::numeric_limits<float>::max();
    for (int i = 0; i < MAX_POPUP_SLOTS; ++i) {
        if (_slots[i].timer <= 0.f) { freeIdx = i; break; }
        if (_slots[i].timer < minTimer) { minTimer = _slots[i].timer; oldestIdx = i; }
    }
    int idx = (freeIdx >= 0) ? freeIdx : oldestIdx;

    _slots[idx].provider->set(title, subtitle, color);
    _slots[idx].timer = dur;
    if (_slots[idx].container)
        _slots[idx].container->setVisible(true, POPUP_ANIM);
    repack();
}

void PopupPanel::tick(float dt)
{
    bool changed = false;
    for (auto& slot : _slots) {
        if (slot.timer <= 0.f) continue;
        slot.timer -= dt;
        if (slot.timer <= 0.f) {
            slot.timer = 0.f;
            if (slot.container) slot.container->setVisible(false, POPUP_ANIM);
            changed = true;
        }
    }
    if (changed) repack();
}

void PopupPanel::repack()
{
    int pos = 0;
    for (int i = 0; i < MAX_POPUP_SLOTS; ++i) {
        if (_slots[i].timer <= 0.f) continue;
        float offsetY = -static_cast<float>(pos) * POPUP_SLOT_HEIGHT;
        if (_slots[i].container)
            _slots[i].container->setAnchorOffset({0.f, offsetY});
        ++pos;
    }
}

} // namespace zappy
