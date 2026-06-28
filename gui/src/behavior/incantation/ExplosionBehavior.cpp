#include "ExplosionBehavior.hpp"
#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include "event/RenderEvent.hpp"
#include <cmath>
#include <algorithm>

namespace behavior {

float ExplosionBehavior::fhash(float a, float b)
{
    float v = std::sin(a * 127.1f + b * 311.7f) * 43758.5453f;
    return v - std::floor(v);
}

ExplosionBehavior::ExplosionBehavior(zappy::EntityManager& entities,
                                     graphic::EntityID selfId,
                                     graphic::Vector3f center,
                                     graphic::Vector3f surfaceNormal,
                                     graphic::Color4b color,
                                     float duration,
                                     float maxRadius,
                                     float maxHeight,
                                     float startDelay)
    : _entities(&entities), _selfId(selfId),
      _center(center), _surfaceNormal(surfaceNormal),
      _color(color), _duration(duration), _maxRadius(maxRadius),
      _maxHeight(maxHeight), _startDelay(startDelay)
{
}

// ---- helpers ---------------------------------------------------------------

static graphic::Vector3f cross3(graphic::Vector3f a, graphic::Vector3f b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

static graphic::Vector3f normalize3(graphic::Vector3f v) {
    float l = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if (l < 1e-5f) return {0.f, 1.f, 0.f};
    return {v.x/l, v.y/l, v.z/l};
}

// Build two vectors perpendicular to `n`
static void buildTangentFrame(graphic::Vector3f n,
                               graphic::Vector3f& right,
                               graphic::Vector3f& fwd)
{
    graphic::Vector3f up = std::abs(n.y) > 0.99f
                         ? graphic::Vector3f{1.f, 0.f, 0.f}
                         : graphic::Vector3f{0.f, 1.f, 0.f};
    right = normalize3(cross3(n, up));
    fwd   = normalize3(cross3(right, n));
}

// Rotate vector `v` around `axis` (unit) by `angle` radians (Rodrigues)
static graphic::Vector3f rotateAround(graphic::Vector3f v,
                                      graphic::Vector3f axis,
                                      float angle)
{
    float c = std::cos(angle), s = std::sin(angle);
    auto dot = [](graphic::Vector3f a, graphic::Vector3f b){ return a.x*b.x+a.y*b.y+a.z*b.z; };
    float d = dot(v, axis);
    auto cr  = cross3(axis, v);
    return {
        v.x*c + cr.x*s + axis.x*d*(1-c),
        v.y*c + cr.y*s + axis.y*d*(1-c),
        v.z*c + cr.z*s + axis.z*d*(1-c),
    };
}

// ---- init ------------------------------------------------------------------

void ExplosionBehavior::initRockets()
{
    graphic::Vector3f right, fwd;
    buildTangentFrame(_surfaceNormal, right, fwd);

    float id = static_cast<float>(_selfId);
    _rockets.resize(N_ROCKETS);

    for (int i = 0; i < N_ROCKETS; ++i) {
        float fi = static_cast<float>(i);
        // Random azimuth spread around surface normal
        float azimuth = fhash(fi, id)           * 2.f * 3.14159f;
        float tilt    = fhash(fi + 0.5f, id+1.f) * SPREAD_ANGLE;

        // Build a direction tilted `tilt` radians from surfaceNormal
        // by rotating in the tangent plane at angle `azimuth`
        graphic::Vector3f tiltAxis = {
            right.x * std::cos(azimuth) + fwd.x * std::sin(azimuth),
            right.y * std::cos(azimuth) + fwd.y * std::sin(azimuth),
            right.z * std::cos(azimuth) + fwd.z * std::sin(azimuth),
        };
        tiltAxis = normalize3(tiltAxis);
        graphic::Vector3f dir = normalize3(rotateAround(_surfaceNormal, tiltAxis, tilt));

        float height = _maxHeight * (0.55f + fhash(fi + 2.3f, id + 3.f) * 0.55f);

        _rockets[i].dir    = dir;
        _rockets[i].pos    = _center;
        _rockets[i].height = height;
        _rockets[i].burst  = false;
        _rockets[i].burstCenter = {
            _center.x + dir.x * height,
            _center.y + dir.y * height,
            _center.z + dir.z * height,
        };
    }
}

void ExplosionBehavior::initSparks(Rocket& rocket, int rocketIndex)
{
    float burstDuration = _duration * (1.f - RISE_FRAC);
    float id  = static_cast<float>(_selfId);
    float ri  = static_cast<float>(rocketIndex);

    rocket.sparks.resize(N_SPARKS);

    for (int i = 0; i < N_SPARKS; ++i) {
        float fi = static_cast<float>(i);

        // Different frequency multipliers per component break Lissajous degeneracy
        float seed = fi + id * 0.37f + ri * 17.f;
        float dx = fhash(seed * 1.00f, ri + 3.f) * 2.f - 1.f;
        float dy = fhash(seed * 1.61f, ri + 7.f) * 2.f - 1.f;
        float dz = fhash(seed * 2.39f, ri + 11.f) * 2.f - 1.f;
        graphic::Vector3f dir = normalize3({ dx, dy, dz });

        float speedFactor = 0.55f + fhash(fi + 0.7f, id + ri) * 0.5f;
        float speed = (_maxRadius / burstDuration) * speedFactor * 1.8f;

        auto tint = [](graphic::Color4b c, float t) -> graphic::Color4b {
            return {
                static_cast<uint8_t>(std::min(255.f, c.r * t)),
                static_cast<uint8_t>(std::min(255.f, c.g * t)),
                static_cast<uint8_t>(std::min(255.f, c.b * t)),
                c.a
            };
        };
        float brightness = 0.85f + fhash(fi + 1.1f, id + ri) * 0.3f;

        rocket.sparks[i] = {
            .pos      = rocket.burstCenter,
            .vel      = { dir.x * speed, dir.y * speed, dir.z * speed },
            .velDir   = dir,
            .color    = tint(_color, brightness),
            .tailColor = {},
            .life     = 1.f,
            .speed    = speed,
            .trailLen = 0.f,
        };
    }
    rocket.burst = true;
}

// ---- update ----------------------------------------------------------------

void ExplosionBehavior::onUpdate(graphic::Entity&, float dt)
{
    if (_done) return;

    if (_waitElapsed < _startDelay) {
        _waitElapsed += dt;
        return;
    }

    _elapsed += dt;

    if (_elapsed >= _duration) {
        _done = true;
        _entities->removeEntity(_selfId);
        return;
    }

    float riseDuration  = _duration * RISE_FRAC;
    float burstDuration = _duration * (1.f - RISE_FRAC);

    if (_phase == Phase::Waiting) {
        initRockets();
        _phase = Phase::Rising;
    }

    if (_phase == Phase::Rising) {
        float t      = std::min(_elapsed / riseDuration, 1.f);
        float tEased = t * t * (3.f - 2.f * t);

        for (auto& rocket : _rockets) {
            rocket.pos = {
                _center.x + rocket.dir.x * tEased * rocket.height,
                _center.y + rocket.dir.y * tEased * rocket.height,
                _center.z + rocket.dir.z * tEased * rocket.height,
            };
        }

        if (_elapsed >= riseDuration) {
            for (int i = 0; i < N_ROCKETS; ++i)
                initSparks(_rockets[i], i);
            _phase = Phase::Bursting;
            if (_onBurst) _onBurst(_center);
        }
    } else {
        float sparkLife  = burstDuration;
        float drag       = 1.f - 0.35f * dt;
        float gravX      = _surfaceNormal.x * GRAVITY * dt;
        float gravY      = _surfaceNormal.y * GRAVITY * dt;
        float gravZ      = _surfaceNormal.z * GRAVITY * dt;
        float lifeNorm   = (_elapsed - riseDuration) / sparkLife;
        for (auto& rocket : _rockets) {
            for (auto& s : rocket.sparks) {
                s.pos.x += s.vel.x * dt;
                s.pos.y += s.vel.y * dt;
                s.pos.z += s.vel.z * dt;
                s.vel.x = (s.vel.x - gravX) * drag;
                s.vel.y = (s.vel.y - gravY) * drag;
                s.vel.z = (s.vel.z - gravZ) * drag;
                s.life = std::max(0.f, 1.f - lifeNorm);

                float vl = s.vel.x*s.vel.x + s.vel.y*s.vel.y + s.vel.z*s.vel.z;
                if (vl > 1e-6f) {
                    float inv = 1.f / std::sqrt(vl);
                    s.velDir = { s.vel.x * inv, s.vel.y * inv, s.vel.z * inv };
                }
                s.trailLen = std::min(s.speed * 0.10f * s.life, _maxRadius * 0.30f);

                float life2 = s.life * s.life;
                s.tailColor = {
                    static_cast<uint8_t>(s.color.r / 3),
                    static_cast<uint8_t>(s.color.g / 3),
                    static_cast<uint8_t>(s.color.b / 3),
                    static_cast<uint8_t>(std::clamp(life2 * 0.25f * 255.f, 0.f, 255.f))
                };
            }
        }
    }
}

// ---- render ----------------------------------------------------------------

void ExplosionBehavior::renderRising(graphic::IRenderer& r) const
{
    auto alpha = [](float a) -> uint8_t {
        return static_cast<uint8_t>(std::min(255.f, a * 255.f));
    };

    for (const auto& rocket : _rockets) {
        // Short trail: segment from (head - dir*TRAIL_LEN) to head
        graphic::Vector3f tail = {
            rocket.pos.x - rocket.dir.x * TRAIL_LEN,
            rocket.pos.y - rocket.dir.y * TRAIL_LEN,
            rocket.pos.z - rocket.dir.z * TRAIL_LEN,
        };

        graphic::Color4b orange = { 255, 140,  30, alpha(0.7f) };
        graphic::Color4b dim    = { 200,  80,  10, alpha(0.35f) };
        r.drawLine3D(tail, rocket.pos, dim);
        r.drawLine3D(tail, rocket.pos, orange);

        // Small bright sphere at head
        graphic::Color4b white = { 255, 240, 200, alpha(0.95f) };
        r.drawSphere3D(rocket.pos, 0.06f, white);
        r.drawSphere3D(rocket.pos, 0.035f, { 255, 255, 230, alpha(1.f) });
    }
}

void ExplosionBehavior::renderBurst(graphic::IRenderer& r) const
{
    for (const auto& rocket : _rockets) {
        for (const auto& s : rocket.sparks) {
            if (s.life <= 0.01f) continue;

            float life2 = s.life * s.life;

            graphic::Vector3f tail = {
                s.pos.x - s.velDir.x * s.trailLen,
                s.pos.y - s.velDir.y * s.trailLen,
                s.pos.z - s.velDir.z * s.trailLen,
            };

            graphic::Color4b headCol = {
                s.color.r, s.color.g, s.color.b,
                static_cast<uint8_t>(std::clamp(life2 * 255.f, 0.f, 255.f))
            };

            r.drawLine3D(tail, s.pos, s.tailColor);
            r.drawLine3D(tail, s.pos, headCol);

            if (s.life > 0.25f) {
                float headSize = 0.055f * s.life;
                r.drawSphere3D(s.pos, headSize, { 255, 255, 220,
                    static_cast<uint8_t>(std::clamp(life2 * 0.85f * 255.f, 0.f, 255.f)) });
            }
        }
    }
}

void ExplosionBehavior::onEvent(graphic::Entity&, const event::Event& ev)
{
    event::on(ev,
        [&](const event::RenderEvent& re) {
            if (_done || _phase == Phase::Waiting) return;
            if (_phase == Phase::Rising)
                renderRising(re.renderer);
            else
                renderBurst(re.renderer);
        }
    );
}

} // namespace behavior
