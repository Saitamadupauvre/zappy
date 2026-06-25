#pragma once
#include "logger/Logger.hpp"
#include "scene/IScene.hpp"
#include "graphic/IRenderer.hpp"

namespace zappy {

class Locator
{

    public:
        static void provide(Logger* logger) { _logger = logger; }
        static void provide(IScene* scene) { _scene = scene; }
        static void provide(graphic::IRenderer* renderer) { _renderer = renderer; }

        static Logger* getLogger() { return _logger; }
        static IScene* getScene() { return _scene; }

        static graphic::IRenderer* getRenderer() { return _renderer; }


    private:
        static Logger* _logger;
        static IScene* _scene;
        static graphic::IRenderer* _renderer;
};

} // namespace zappy