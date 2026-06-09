#pragma once

#include "Scene.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"

namespace zappy {

class TestCubeScene : public Scene
{
    public:
        TestCubeScene(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory);
        ~TestCubeScene() override = default;

        void update(const World& world) override;
};

} // namespace zappy
