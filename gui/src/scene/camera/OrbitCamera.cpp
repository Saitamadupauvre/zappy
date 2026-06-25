#include "OrbitCamera.hpp"
#include "event/Event.hpp"
#include <cmath>
#include <algorithm>

static constexpr float DEG_TO_RAD = static_cast<float>(M_PI) / 180.0f;
static constexpr float RAD_TO_DEG = 180.0f / static_cast<float>(M_PI);
static constexpr float FPS_SPEED  = 12.0f;
static constexpr float FOLLOW_LERP = 8.0f;

namespace zappy {

// ── helpers ──────────────────────────────────────────────────────────────────

graphic::Vector3f OrbitCamera::orbitPosition(const graphic::Vector3f& orbitTarget) const
{
    float pr = pitch * DEG_TO_RAD;
    float yr = yaw   * DEG_TO_RAD;
    return {
        orbitTarget.x + distance * std::cos(pr) * std::sin(yr),
        orbitTarget.y + distance * std::sin(pr),
        orbitTarget.z + distance * std::cos(pr) * std::cos(yr)
    };
}

graphic::Vector3f OrbitCamera::fpsForward() const
{
    float pr = _fpsPitch * DEG_TO_RAD;
    float yr = _fpsYaw   * DEG_TO_RAD;
    return {
        std::cos(pr) * std::sin(yr),
        std::sin(pr),
        std::cos(pr) * std::cos(yr)
    };
}

// ── state transitions ─────────────────────────────────────────────────────────

void OrbitCamera::setMapCenter(const graphic::Vector3f& center)
{
    _centerTarget = center;
}

void OrbitCamera::enterCenter()
{
    _mode = Mode::Center;
}

void OrbitCamera::enterFollow(const graphic::Vector3f& pos)
{
    if (_mode != Mode::Follow)
        _prevMode = _mode;
    _followTarget = pos;
    _mode = Mode::Follow;
}

void OrbitCamera::exitFollow()
{
    if (_prevMode == Mode::Free)
        _mode = Mode::Free;
    else
        enterCenter();
}

void OrbitCamera::enterFirstPerson(const graphic::Vector3f& pos, const graphic::Vector3f& fwd, const graphic::Vector3f& up)
{
    _fpvPos = pos;
    _fpvFwd = fwd;
    _fpvUp  = up;
    _prevMode = (_mode == Mode::FirstPerson) ? _prevMode : _mode;
    _mode = Mode::FirstPerson;
}

void OrbitCamera::updateFirstPersonTarget(const graphic::Vector3f& pos, const graphic::Vector3f& fwd, const graphic::Vector3f& up)
{
    _fpvPos = pos;
    _fpvFwd = fwd;
    _fpvUp  = up;
}

void OrbitCamera::enterFree()
{
    // Convert current camera position + look direction into FPS state
    const graphic::Vector3f& orbitTgt = (_mode == Mode::Follow) ? _followTarget : _centerTarget;
    graphic::Vector3f pos = orbitPosition(orbitTgt);

    _fpsPos = pos;
    // look direction from orbit pos toward orbit target
    graphic::Vector3f dir = {
        orbitTgt.x - pos.x,
        orbitTgt.y - pos.y,
        orbitTgt.z - pos.z
    };
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    if (len > 0.0001f) {
        dir.x /= len; dir.y /= len; dir.z /= len;
    }
    _fpsPitch = std::asin(std::clamp(dir.y, -1.0f, 1.0f)) * RAD_TO_DEG;
    _fpsYaw   = std::atan2(dir.x, dir.z) * RAD_TO_DEG;

    _mode = Mode::Free;
}

void OrbitCamera::updateFollowTarget(const graphic::Vector3f& pos, float dt)
{
    float t = std::min(1.0f, FOLLOW_LERP * dt);
    _followTarget.x += (pos.x - _followTarget.x) * t;
    _followTarget.y += (pos.y - _followTarget.y) * t;
    _followTarget.z += (pos.z - _followTarget.z) * t;
}

// ── camera state ─────────────────────────────────────────────────────────────

graphic::CameraState OrbitCamera::toCameraState() const
{
    if (_mode == Mode::FirstPerson) {
        graphic::Vector3f tgt = {
            _fpvPos.x + _fpvFwd.x,
            _fpvPos.y + _fpvFwd.y,
            _fpvPos.z + _fpvFwd.z
        };
        return { _fpvPos, tgt, _fpvUp, fov };
    }

    if (_mode == Mode::Free) {
        auto fwd = fpsForward();
        graphic::Vector3f tgt = {
            _fpsPos.x + fwd.x,
            _fpsPos.y + fwd.y,
            _fpsPos.z + fwd.z
        };
        return { _fpsPos, tgt, graphic::Vector3f::up(), fov };
    }

    const graphic::Vector3f& orbitTgt =
        (_mode == Mode::Follow) ? _followTarget : _centerTarget;
    return { orbitPosition(orbitTgt), orbitTgt, graphic::Vector3f::up(), fov };
}

// ── events ────────────────────────────────────────────────────────────────────

void OrbitCamera::handleEvent(const event::Event& ev)
{
    event::on(ev,
        [&](const event::MouseWheelEvent& e) {
            if (_mode == Mode::FirstPerson) return;
            if (_mode == Mode::Free) {
                // zoom = move forward/backward
                auto fwd = fpsForward();
                float spd = e.delta * 1.5f;
                _fpsPos.x += fwd.x * spd;
                _fpsPos.y += fwd.y * spd;
                _fpsPos.z += fwd.z * spd;
            } else {
                distance -= e.delta * 0.8f;
                if (distance < 1.0f)   distance = 1.0f;
                if (distance > 100.0f) distance = 100.0f;
            }
        },
        [&](const event::MouseMoveEvent& e) {
            if (!_rightHeld || _mode == Mode::FirstPerson) return;
            if (_mode == Mode::Free) {
                _fpsYaw   -= e.delta.x * 0.3f;
                _fpsPitch -= e.delta.y * 0.3f;
                if (_fpsPitch >  89.0f) _fpsPitch =  89.0f;
                if (_fpsPitch < -89.0f) _fpsPitch = -89.0f;
            } else {
                yaw   -= e.delta.x * 0.3f;
                pitch += e.delta.y * 0.3f;
                if (pitch >  89.0f) pitch =  89.0f;
                if (pitch < -89.0f) pitch = -89.0f;
            }
        },
        [&](const event::MouseButtonEvent& e) {
            if (e.button == graphic::MouseBtn::RIGHT)
                _rightHeld = e.pressed;
        }
    );
}

// ── update ────────────────────────────────────────────────────────────────────

void OrbitCamera::update(float dt, const InputManager& input)
{
    if (_mode != Mode::Free) return;

    float yr  = _fpsYaw * DEG_TO_RAD;
    float spd = FPS_SPEED * dt;

    // Full 3D forward (includes pitch for flying camera)
    auto  fwd3d = fpsForward();
    // Right = cross(fwd_xz, up) in Raylib's right-handed coord (+X right, +Z toward viewer)
    graphic::Vector3f right{ -std::cos(yr), 0.0f, std::sin(yr) };

    if (input.isActionActive(InputAction::MOVE_FORWARD)) {
        _fpsPos.x += fwd3d.x * spd;
        _fpsPos.y += fwd3d.y * spd;
        _fpsPos.z += fwd3d.z * spd;
    }
    if (input.isActionActive(InputAction::MOVE_BACKWARD)) {
        _fpsPos.x -= fwd3d.x * spd;
        _fpsPos.y -= fwd3d.y * spd;
        _fpsPos.z -= fwd3d.z * spd;
    }
    if (input.isActionActive(InputAction::MOVE_RIGHT)) {
        _fpsPos.x += right.x * spd;
        _fpsPos.z += right.z * spd;
    }
    if (input.isActionActive(InputAction::MOVE_LEFT)) {
        _fpsPos.x -= right.x * spd;
        _fpsPos.z -= right.z * spd;
    }
}

} // namespace zappy
