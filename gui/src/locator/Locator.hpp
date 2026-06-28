#pragma once
#include "logger/Logger.hpp"
#include "scene/IScene.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"

namespace zappy {

class Locator
{

    public:
        static void provide(Logger* logger) { _logger = logger; }
        static void provide(IScene* scene) { _scene = scene; }
        static void provide(graphic::IRenderer* renderer) { _renderer = renderer; }
        static void provideDefaultFont(graphic::FontHandle font) { _defaultFont = font; }
        static void provideCjkFont(graphic::FontHandle font)     { _cjkFont = font; }

        static Logger* getLogger() { return _logger; }
        static IScene* getScene() { return _scene; }
        static graphic::IRenderer* getRenderer() { return _renderer; }
        static graphic::FontHandle getDefaultFont() { return _defaultFont; }
        static graphic::FontHandle getCjkFont()     { return _cjkFont; }

    private:
        static Logger* _logger;
        static IScene* _scene;
        static graphic::IRenderer* _renderer;
        static graphic::FontHandle _defaultFont;
        static graphic::FontHandle _cjkFont;
};

} // namespace zappy