#pragma once

#include "scene/IScene.hpp"
#include "scene/camera/OrbitCamera.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "core/manager/input/InputManager.hpp"
#include "behavior/hud/HudPicker.hpp"
#include "behavior/pick/PickSystem.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"
#include "graphic/IRenderer.hpp"

namespace zappy {

class Scene : public IScene
{
    public:

        Scene();
        ~Scene() override = default;

        void render(graphic::IRenderer& renderer) override;
        void handleEvent(const event::Event& event) override;

        void update(const World& world, float dt) override = 0;

        HudManager& getHud() override { return _hud; };
        EntityManager& getEntityManager() override { return _entities; }
        OrbitCamera&   getCamera()         { return _camera; }
        InputManager&  getInputManager()   { return _inputManager; }

    protected:
        OrbitCamera   _camera;
        EntityManager _entities;
        HudManager    _hud;
        InputManager  _inputManager;
        HudPicker     _hudPicker;
        PickSystem     _pickSystem;

    protected:
        graphic::IRenderer* _renderer       = nullptr;

    private:
        ContextLogger      _log{"Scene"};
        graphic::Vector2f  _lastMousePos    = {0.f, 0.f};

        void setupDefaultInputs();
};

} // namespace zappy
