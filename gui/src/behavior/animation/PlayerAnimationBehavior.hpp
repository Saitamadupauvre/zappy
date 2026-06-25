#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"
#include <cstdint>
#include <unordered_map>

namespace behavior {

// Drives the player model's skeletal animation: idle while standing, run while
// moving. Run playback spans the move duration so its speed tracks the server
// frequency (move duration = 7 ticks / timeUnit, see AnimationClock).
class PlayerAnimationBehavior : public ABehavior {
public:
    PlayerAnimationBehavior(graphic::IRenderer& renderer, graphic::ModelHandle model,
                            uint32_t entityId);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

    void setModel(graphic::ModelHandle model) { _model = model; _frame = 0.0f; }

private:
    // GLB animation order in player_lv1.glb: 0 = "Idle.003", 1 = "Run.002", 2 = "Take", 3 = "Dance".
    static constexpr int   IDLE_ANIM   = 0;
    static constexpr int   RUN_ANIM    = 1;
    static constexpr int   TAKE_ANIM   = 2;
    static constexpr int   DANCE_ANIM  = 3;

    static constexpr float NATURAL_FPS = 24.0f;

    static const std::unordered_map<int, float> ANIM_SPEED_SCALE;

    void setAnim(int index);

    graphic::IRenderer*  _renderer;
    graphic::ModelHandle _model;
    uint32_t             _entityId;

    int   _animIndex    = IDLE_ANIM;
    float _frame        = 0.0f;
    float _moveDuration = 0.0f;
    int   _timeUnit     = 1;
    bool  _running      = false;
    bool  _takePlaying  = false;
    bool  _dancing      = false;
    int   _incantX      = -1;
    int   _incantY      = -1;
};

} // namespace behavior
