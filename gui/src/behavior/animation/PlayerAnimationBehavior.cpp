#include "PlayerAnimationBehavior.hpp"
#include "behavior/movement/MovementBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <algorithm>
#include <cmath>

namespace behavior {

const std::unordered_map<int, float> PlayerAnimationBehavior::ANIM_SPEED_SCALE = {
    {IDLE_ANIM,  .5f},
    {RUN_ANIM,   1.0f},
    {TAKE_ANIM,  1.0f},
    {DANCE_ANIM, .5f},
};

PlayerAnimationBehavior::PlayerAnimationBehavior(graphic::IRenderer& renderer,
                                                 graphic::ModelHandle model,
                                                 uint32_t entityId)
    : _renderer(&renderer), _model(model), _entityId(entityId) {}

void PlayerAnimationBehavior::setAnim(int index)
{
    if (index == _animIndex) return;
    _animIndex = index;
    _frame     = 0.0f;
}

void PlayerAnimationBehavior::onUpdate(graphic::Entity& owner, float dt)
{
    auto move = owner.getBehavior<MovementBehavior>();
    if (_running && (!move || !move->isMoving())) {
        _running = false;
        if (!_takePlaying && !_dancing)
            setAnim(IDLE_ANIM);
    }

    int frames = _renderer->modelAnimationFrameCount(_model, _animIndex);
    if (frames <= 0) return;

    auto   it         = ANIM_SPEED_SCALE.find(_animIndex);
    float  speedScale = (it != ANIM_SPEED_SCALE.end()) ? it->second : 1.0f;
    float  scaledFps  = NATURAL_FPS * static_cast<float>(_timeUnit) * speedScale;
    float  fps        = scaledFps;
    if (_running && _moveDuration > 0.0f)
        fps = static_cast<float>(frames) / _moveDuration;

    _frame += fps * dt;

    if (_takePlaying) {
        if (_frame >= static_cast<float>(frames)) {
            _takePlaying = false;
            _frame = 0.0f;
            setAnim(_dancing ? DANCE_ANIM : IDLE_ANIM);
        } else {
            _renderer->updateModelAnimation(_model, _animIndex, _frame);
        }
        return;
    }

    _frame = std::fmod(_frame, static_cast<float>(frames));
    _renderer->updateModelAnimation(_model, _animIndex, _frame);
}

void PlayerAnimationBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::EntityMoveToEvent& e) {
            if (e.entityId != _entityId || e.duration <= 0.0f) return;
            _moveDuration = e.duration;
            _running      = true;
            setAnim(RUN_ANIM);
        },
        [&](const event::ResourceCollectedEvent& e) {
            if (e.playerId != _entityId) return;
            _takePlaying = true;
            setAnim(TAKE_ANIM);
        },
        [&](const event::ResourceDroppedEvent& e) {
            if (e.playerId != _entityId) return;
            _takePlaying = true;
            setAnim(TAKE_ANIM);
        },
        [&](const event::IncantationStartEvent& e) {
            auto it = std::find(e.playerIds.begin(), e.playerIds.end(), _entityId);
            if (it == e.playerIds.end()) return;
            _dancing  = true;
            _incantX  = e.x;
            _incantY  = e.y;
            if (!_takePlaying && !_running)
                setAnim(DANCE_ANIM);
        },
        [&](const event::TimeUnitChangedEvent& e) {
            _timeUnit = e.timeUnit;
        },
        [&](const event::IncantationEndEvent& e) {
            if (!_dancing || e.x != _incantX || e.y != _incantY) return;
            _dancing = false;
            _incantX = -1;
            _incantY = -1;
            if (!_takePlaying && !_running)
                setAnim(IDLE_ANIM);
        }
    );
}

} // namespace behavior
