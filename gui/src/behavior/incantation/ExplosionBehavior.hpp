#pragma once

#include "behavior/ABehavior.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "graphic/IRenderer.hpp"
#include <vector>
#include <cstdint>

namespace behavior {

class ExplosionBehavior : public ABehavior
{
public:
    ExplosionBehavior(zappy::EntityManager& entities,
                      graphic::EntityID selfId,
                      graphic::Vector3f center,
                      graphic::Vector3f surfaceNormal,
                      graphic::Color4b color,
                      float duration,
                      float maxRadius,
                      float maxHeight,
                      float startDelay = 0.f);

    void onUpdate(graphic::Entity& owner, float dt) override;
    void onEvent(graphic::Entity& owner, const event::Event& ev) override;

private:
    struct Spark {
        graphic::Vector3f pos;
        graphic::Vector3f vel;
        graphic::Vector3f velDir; // normalized vel, updated in onUpdate
        graphic::Color4b  color;
        graphic::Color4b  tailColor;
        float             life     = 1.f;
        float             speed    = 1.f;
        float             trailLen = 0.f;
    };

    struct Rocket {
        graphic::Vector3f dir;         // normalized launch direction
        graphic::Vector3f pos;         // current head position
        graphic::Vector3f burstCenter; // where sparks spawn from
        std::vector<Spark> sparks;
        float height = 1.f;            // individual max height
        bool burst = false;
    };

    enum class Phase { Waiting, Rising, Bursting };

    void initRockets();
    void initSparks(Rocket& rocket, int rocketIndex);
    void renderRising(graphic::IRenderer& r) const;
    void renderBurst(graphic::IRenderer& r) const;

    static float fhash(float a, float b);

    zappy::EntityManager* _entities;
    graphic::EntityID     _selfId;
    graphic::Vector3f     _center;
    graphic::Vector3f     _surfaceNormal;
    graphic::Color4b      _color;
    float                 _duration;
    float                 _maxRadius;
    float                 _maxHeight;
    float                 _startDelay;

    float  _elapsed      = 0.f;
    float  _waitElapsed  = 0.f;
    bool   _done         = false;
    Phase  _phase        = Phase::Waiting;

    std::vector<Rocket> _rockets;

    static constexpr float RISE_FRAC    = 0.22f;
    static constexpr int   N_ROCKETS    = 3;
    static constexpr int   N_SPARKS     = 12;
    static constexpr float GRAVITY      = 14.f;
    static constexpr float SPREAD_ANGLE = 0.38f;
    static constexpr float TRAIL_LEN    = 0.55f;
};

} // namespace behavior
