#pragma once

#include "scene/IScene.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"

namespace zappy {

class Scene : public IScene
{
    public:
        Scene() = default;
        ~Scene() override = default;

        void render(graphic::IRenderer& renderer) override;
        void handleEvent(const event::Event& event) override;

        void update(const World& world) override = 0;

    protected:
        struct OrbitCamera {
            float yaw      = 0.0f;
            float pitch    = 30.0f;
            float distance = 8.0f;
            float fov      = 45.0f;
            graphic::Vector3f target{0.0f, 0.0f, 0.0f};

            graphic::Vector3f position() const;
        };

        OrbitCamera   _camera;
        EntityManager _entities;
        HudManager    _hud;

    private:
        bool _rightButtonHeld = false;
};

} // namespace zappy
