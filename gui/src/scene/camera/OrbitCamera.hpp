#pragma once

#include "event/Event.hpp"
#include "graphic/Types.hpp"
#include "core/manager/input/InputManager.hpp"

namespace zappy {

class OrbitCamera {
public:
    enum class Mode { Center, Follow, Free, FirstPerson };

    // Shared orbit params (Center + Follow modes)
    float yaw      = 0.0f;
    float pitch    = 30.0f;
    float distance = 8.0f;
    float fov      = 45.0f;

    void handleEvent(const event::Event& ev);
    void update(float dt, const InputManager& input);

    // State transitions
    void setMapCenter(const graphic::Vector3f& center);
    void enterCenter();
    void enterFollow(const graphic::Vector3f& pos);
    void exitFollow();
    void enterFree();

    // Follow update — call each frame with entity's actual interpolated position
    void updateFollowTarget(const graphic::Vector3f& pos, float dt);

    void enterFirstPerson(const graphic::Vector3f& pos, const graphic::Vector3f& fwd, const graphic::Vector3f& up);
    void updateFirstPersonTarget(const graphic::Vector3f& pos, const graphic::Vector3f& fwd, const graphic::Vector3f& up);

    bool isFollowing()    const { return _mode == Mode::Follow; }
    bool isFirstPerson()  const { return _mode == Mode::FirstPerson; }
    Mode getMode()        const { return _mode; }

    graphic::CameraState toCameraState() const;

private:
    graphic::Vector3f orbitPosition(const graphic::Vector3f& orbitTarget) const;
    graphic::Vector3f fpsForward() const;

    Mode _mode     = Mode::Center;
    Mode _prevMode = Mode::Center;
    bool _rightHeld = false;

    // Center mode
    graphic::Vector3f _centerTarget{0.0f, 0.0f, 0.0f};

    // Follow mode
    graphic::Vector3f _followTarget{0.0f, 0.0f, 0.0f};

    // Free FPS mode
    graphic::Vector3f _fpsPos{0.0f, 5.0f, 0.0f};
    float _fpsPitch = -20.0f;
    float _fpsYaw   =   0.0f;

    // First-person player view mode
    graphic::Vector3f _fpvPos{};
    graphic::Vector3f _fpvFwd{};
    graphic::Vector3f _fpvUp{};
};

} // namespace zappy
