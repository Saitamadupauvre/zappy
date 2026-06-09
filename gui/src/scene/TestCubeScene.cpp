#include "TestCubeScene.hpp"
#include "behavior/drawable/mesh/MeshDrawableBehavior.hpp"
#include "behavior/drawable/hud/HudInfoBehavior.hpp"
#include "graphic/Types.hpp"

namespace zappy {

TestCubeScene::TestCubeScene(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory)
{
    auto cubeData = meshFactory.createCube(1.0f, 1.0f, 1.0f);
    auto cubeMesh = renderer.uploadMesh(cubeData);

    auto entity = _entities.createEntity(0, "cube");
    entity->addBehavior<behavior::MeshDrawableBehavior>(cubeMesh, graphic::TextureHandle{0});

    auto hudEntity = _hud.createEntity(1, "hud_info");
    auto hud = hudEntity->addBehavior<behavior::HudInfoBehavior>();
    hud->setPosition({10.f, 10.f});
    hud->setLines({
        { "Zappy GUI - Test Scene", graphic::Color4b{255, 220, 50,  255} },
        { "Entity: cube",           graphic::Color4b::white() },
        { "Camera: orbit",          graphic::Color4b{180, 180, 180, 255} },
        { "RMB drag: rotate",       graphic::Color4b{140, 140, 140, 255} },
        { "Scroll: zoom",           graphic::Color4b{140, 140, 140, 255} },
    });
}

void TestCubeScene::update(const World&)
{
}

} // namespace zappy
