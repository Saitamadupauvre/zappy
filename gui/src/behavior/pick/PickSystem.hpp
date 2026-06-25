#pragma once
#include "event/Event.hpp"
#include "graphic/IRenderer.hpp"
#include "entity/Entity.hpp"

namespace zappy {

class Scene;

class PickSystem {
public:
    PickSystem() = default;
    ~PickSystem() = default;

    /**
     * @brief Tente de gérer le clic 3D.
     * @return true si une entité 3D a été touchée et l'événement consommé, false sinon.
     */
    bool tryHandleClick(Scene& scene, graphic::IRenderer& renderer,
                        const event::MouseButtonEvent& e);

    /**
     * @brief Tente de gérer le survol 3D (pour la surbrillance/hover).
     * @return true si une entité 3D est survolée, false sinon.
     */
    bool tryHandleMouseMove(Scene& scene, graphic::IRenderer& renderer,
                            const graphic::Vector2f& mousePos);

private:
    graphic::Entity* raycast(Scene& scene, graphic::IRenderer& renderer,
                             const graphic::Vector2f& mousePos);

    static bool spherePreCull(const graphic::Vector3f& rayOrigin,
                              const graphic::Vector3f& rayDir,
                              const graphic::Vector3f& center, float radius);

    graphic::Vector2f _lastMousePos{-1.f, -1.f};

    static constexpr float MOUSE_MOVE_THRESHOLD = 2.0f;
};

} // namespace zappy
