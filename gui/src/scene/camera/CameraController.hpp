#pragma once

#include "scene/camera/OrbitCamera.hpp"
#include "scene/layout/IMapLayout.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "logger/ContextLogger.hpp"
#include <chrono>
#include <cstdint>

namespace zappy {

class CameraController {
public:
    explicit CameraController(OrbitCamera& camera) : _camera(camera) {}

    void enterFirstPerson(uint32_t playerId, EntityManager& entities, HudManager& hud);
    void exitFirstPerson(uint32_t playerId, EntityManager& entities, HudManager& hud);
    void onFollowToggle(uint32_t selectedPlayerId, EntityManager& entities, HudManager& hud);

    void applyLayoutFraming(const IMapLayout& layout, int worldW, int worldH);

    void update(float dt, uint32_t selectedPlayerId, EntityManager& entities);

    bool isFirstPersonActive() const { return _firstPersonActive; }

private:
    static constexpr float FPV_EYE_HEIGHT = 0.55f;
    static constexpr long  DOUBLE_TAP_MS  = 400;

    OrbitCamera& _camera;
    bool         _firstPersonActive = false;
    std::chrono::steady_clock::time_point _lastFollowPressTime{};

    ContextLogger _log{"CameraController"};
};

} // namespace zappy
