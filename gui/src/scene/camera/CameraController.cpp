#include "CameraController.hpp"

#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"

#include <chrono>
#include <format>

namespace zappy {

void CameraController::applyLayoutFraming(const IMapLayout& layout, int worldW, int worldH)
{
    _camera.yaw = 0.0f;
    CameraFraming f = layout.cameraFraming(worldW, worldH);
    _camera.distance = f.distance;
    _camera.pitch    = f.pitch;
    _camera.setMapCenter({0.0f, 0.0f, 0.0f});
    _camera.enterCenter();
}

void CameraController::enterFirstPerson(uint32_t playerId, EntityManager& entities, HudManager& hud)
{
    auto entity = entities.getEntity(playerId);
    if (!entity) return;
    auto tb = entity->getBehavior<behavior::TransformBehavior>();
    if (!tb) return;

    graphic::Vector3f up  = tb->getUp();
    graphic::Vector3f fwd = tb->getForward();
    graphic::Vector3f pos = tb->getPosition();
    pos.x += up.x * FPV_EYE_HEIGHT;
    pos.y += up.y * FPV_EYE_HEIGHT;
    pos.z += up.z * FPV_EYE_HEIGHT;

    _camera.enterFirstPerson(pos, fwd, up);
    _firstPersonActive = true;

    if (auto drawable = entity->getBehavior<behavior::ModelDrawableBehavior>())
        drawable->setVisible(false);

    if (auto tagEntity = hud.getEntity(playerId + 10000))
        if (auto hc = tagEntity->getBehavior<behavior::HudContainerBehavior>())
            hc->setVisible(false);

    _log.info(std::format("Camera → first-person player {}", playerId));
}

void CameraController::exitFirstPerson(uint32_t playerId, EntityManager& entities, HudManager& hud)
{
    _firstPersonActive = false;
    _camera.exitFollow();

    if (playerId != 0) {
        if (auto entity = entities.getEntity(playerId))
            if (auto drawable = entity->getBehavior<behavior::ModelDrawableBehavior>())
                drawable->setVisible(true);

        if (auto tagEntity = hud.getEntity(playerId + 10000))
            if (auto hc = tagEntity->getBehavior<behavior::HudContainerBehavior>())
                hc->setVisible(true);
    }
    _log.info("Camera → exit first-person");
}

void CameraController::onFollowToggle(uint32_t selectedPlayerId, EntityManager& entities,
                                      HudManager& hud)
{
    auto now = std::chrono::steady_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastFollowPressTime).count();
    _lastFollowPressTime = now;

    if (_firstPersonActive) {
        exitFirstPerson(selectedPlayerId, entities, hud);
        return;
    }
    if (ms < DOUBLE_TAP_MS && selectedPlayerId != 0) {
        enterFirstPerson(selectedPlayerId, entities, hud);
        return;
    }

    if (_camera.isFollowing()) {
        _camera.exitFollow();
        _log.info("Camera → restore previous mode");
        return;
    }
    if (selectedPlayerId != 0) {
        graphic::Vector3f pos{};
        if (auto entity = entities.getEntity(selectedPlayerId))
            if (auto tb = entity->getBehavior<behavior::TransformBehavior>())
                pos = tb->getPosition();
        _camera.enterFollow(pos);
        _log.info(std::format("Camera → follow player {}", selectedPlayerId));
        return;
    }
    if (_camera.getMode() == OrbitCamera::Mode::Center) {
        _camera.enterFree();
        _log.info("Camera → free FPS mode");
    } else {
        _camera.enterCenter();
        _log.info("Camera → center orbit mode");
    }
}

void CameraController::update(float dt, uint32_t selectedPlayerId,
                               EntityManager& entities)
{
    if (_camera.isFirstPerson() && selectedPlayerId != 0) {
        if (auto entity = entities.getEntity(selectedPlayerId))
            if (auto tb = entity->getBehavior<behavior::TransformBehavior>()) {
                graphic::Vector3f up  = tb->getUp();
                graphic::Vector3f fwd = tb->getForward();
                graphic::Vector3f pos = tb->getPosition();
                pos.x += up.x * FPV_EYE_HEIGHT;
                pos.y += up.y * FPV_EYE_HEIGHT;
                pos.z += up.z * FPV_EYE_HEIGHT;
                _camera.updateFirstPersonTarget(pos, fwd, up);
            }
        return;
    }

    if (_camera.isFollowing() && selectedPlayerId != 0) {
        if (auto entity = entities.getEntity(selectedPlayerId)) {
            if (auto tb = entity->getBehavior<behavior::TransformBehavior>()) {
                _camera.updateFollowTarget(tb->getPosition(), dt);
                return;
            }
        }
        // fallback when entity has no TransformBehavior yet — use tile center
        // (handled by caller via playerTiles if needed)
    }
}

} // namespace zappy
