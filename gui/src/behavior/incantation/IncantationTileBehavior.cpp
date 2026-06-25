#include "IncantationTileBehavior.hpp"
#include "behavior/drawable/mesh/MeshDrawableBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <cmath>
#include <algorithm>

namespace behavior {

static constexpr graphic::Color4b INCANT_COLOR   = { 80, 80, 255, 255};
static constexpr graphic::Color4b SUCCESS_COLOR  = {255, 220,  50, 255};
static constexpr graphic::Color4b FAIL_COLOR     = {255,  50,  50, 255};
static constexpr float            FLASH_DURATION = 0.6f;

IncantationTileBehavior::IncantationTileBehavior(int tileX, int tileY,
                                                 graphic::Color4b baseTint,
                                                 const zappy::AnimationClock& clock)
    : _tileX(tileX), _tileY(tileY), _baseTint(baseTint), _clock(clock) {}

void IncantationTileBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    auto drawable = owner.getBehavior<MeshDrawableBehavior>();
    if (!drawable) return;

    if (_flashing) {
        _flashTimer -= dt;
        if (_flashTimer <= 0.0f) {
            _flashing = false;
            _active   = false;
            drawable->setTint(_baseTint);
        } else {
            float alpha = _flashTimer / FLASH_DURATION;
            graphic::Color4b flash = _flashSuccess ? SUCCESS_COLOR : FAIL_COLOR;
            drawable->setTint({
                static_cast<uint8_t>(_baseTint.r + (flash.r - _baseTint.r) * alpha),
                static_cast<uint8_t>(_baseTint.g + (flash.g - _baseTint.g) * alpha),
                static_cast<uint8_t>(_baseTint.b + (flash.b - _baseTint.b) * alpha),
                255
            });
        }
        return;
    }

    if (!_active) return;
    _time += dt;
    float pulse = 0.5f + 0.5f * std::sin(_time * M_PI * 2.0f);
    drawable->setTint({
        static_cast<uint8_t>(_baseTint.r + (INCANT_COLOR.r - _baseTint.r) * pulse),
        static_cast<uint8_t>(_baseTint.g + (INCANT_COLOR.g - _baseTint.g) * pulse),
        static_cast<uint8_t>(_baseTint.b + (INCANT_COLOR.b - _baseTint.b) * pulse),
        255
    });
}

void IncantationTileBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::IncantationStartEvent& e) {
            if (e.x != _tileX || e.y != _tileY) return;
            _active = true;
            _time   = 0.0f;
        },
        [&](const event::IncantationEndEvent& e) {
            if (e.x != _tileX || e.y != _tileY) return;
            _flashing     = true;
            _flashTimer   = FLASH_DURATION;
            _flashSuccess = e.success;
        }
    );
}

} // namespace behavior
