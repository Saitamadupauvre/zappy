#pragma once

#include "graphic/IWindowContext.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/ITextureLoader.hpp"
#include "graphic/IFontLoader.hpp"
#include "scene/IScene.hpp"
#include <memory>

namespace zappy {

class GraphicsContext
{
public:
    GraphicsContext(
        std::unique_ptr<graphic::IWindowContext> window,
        std::unique_ptr<graphic::IRenderer>      renderer,
        std::unique_ptr<graphic::IMeshFactory>   meshFactory,
        std::unique_ptr<graphic::ITextureLoader> textureLoader,
        std::unique_ptr<graphic::IFontLoader>    fontLoader
    );
    ~GraphicsContext() = default;

    GraphicsContext(const GraphicsContext&)            = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    void  pollAndDispatch(IScene& scene);
    void  beginFrame();
    void  endFrame();
    bool  isOpen()       const;
    void  close();
    float getDeltaTime() const;

    void                     setTargetFps(int fps);
    void                     setFullscreen(bool enable);
    void                     setResolution(int width, int height);
    graphic::Vector2f        getWindowSize()     const;
    graphic::IRenderer&      getRenderer();
    graphic::IMeshFactory&   getMeshFactory();
    graphic::ITextureLoader& getTextureLoader();
    graphic::IFontLoader&    getFontLoader();

private:
    std::unique_ptr<graphic::IWindowContext> _window;
    std::unique_ptr<graphic::IRenderer>      _renderer;
    std::unique_ptr<graphic::IMeshFactory>   _meshFactory;
    std::unique_ptr<graphic::ITextureLoader> _textureLoader;
    std::unique_ptr<graphic::IFontLoader>    _fontLoader;
};

} // namespace zappy
