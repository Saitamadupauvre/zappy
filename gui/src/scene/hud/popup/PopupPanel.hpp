#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/NotificationPopupProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include <array>
#include <limits>
#include <memory>
#include <string>

namespace zappy {

class PopupPanel {
public:
    static constexpr int   MAX_POPUP_SLOTS   = 3;
    static constexpr float POPUP_DURATION    = 3.0f;
    static constexpr float POPUP_ANIM        = 0.35f;
    static constexpr float POPUP_SLOT_HEIGHT = 90.0f;

    // Entity IDs: 9893, 9894, 9895
    static constexpr graphic::EntityID POPUP_BASE_ID = 9893;

    void setup(HudManager& hud, graphic::IRenderer& renderer,
               graphic::ITextureLoader& loader);
    void push(const std::string& title, const std::string& subtitle,
              graphic::Color4b color = {255, 255, 255, 255});
    void tick(float dt);

private:
    void repack();

    struct Slot {
        std::shared_ptr<NotificationPopupProvider>      provider;
        std::shared_ptr<behavior::HudContainerBehavior> container;
        float timer = 0.f;
    };
    std::array<Slot, MAX_POPUP_SLOTS> _slots;
    ContextLogger _log{"PopupPanel"};
};

} // namespace zappy
