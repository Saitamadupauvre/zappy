#include "GraphicsContext.hpp"
#include "event/Event.hpp"

namespace zappy {

GraphicsContext::GraphicsContext(
    std::unique_ptr<graphic::IWindowContext> window,
    std::unique_ptr<graphic::IRenderer>      renderer,
    std::unique_ptr<graphic::IMeshFactory>   meshFactory,
    std::unique_ptr<graphic::ITextureLoader> textureLoader,
    std::unique_ptr<graphic::IFontLoader>    fontLoader
)
    : _window       (std::move(window))
    , _renderer     (std::move(renderer))
    , _meshFactory  (std::move(meshFactory))
    , _textureLoader(std::move(textureLoader))
    , _fontLoader   (std::move(fontLoader))
{}

void GraphicsContext::pollAndDispatch(IScene& scene)
{
    for (const auto& ev : _window->pollEvents()) {
        bool closed = false;
        event::on(ev, [&](const event::WindowClosedEvent&) { closed = true; });
        if (closed) {
            _window->close();
            break;
        }
        scene.handleEvent(ev);
    }
}

void GraphicsContext::beginFrame()
{
    _window->beginFrame();
    _renderer->clear(graphic::Color4b::black());
}

void GraphicsContext::endFrame()
{
    _window->endFrame();
}

bool GraphicsContext::isOpen() const
{
    return _window->isOpen();
}

void GraphicsContext::close()
{
    _window->close();
}

float GraphicsContext::getDeltaTime() const
{
    return _window->getDeltaTime();
}

void GraphicsContext::setTargetFps(int fps)         { _window->setTargetFps(fps); }
void GraphicsContext::setFullscreen(bool enable)    { _window->setFullscreen(enable); }
void GraphicsContext::setResolution(int w, int h)   { _window->setResolution(w, h); }

graphic::Vector2f GraphicsContext::getWindowSize() const
{
    return _window->getSize();
}

graphic::IRenderer& GraphicsContext::getRenderer()
{
    return *_renderer;
}

graphic::IMeshFactory& GraphicsContext::getMeshFactory()
{
    return *_meshFactory;
}

graphic::ITextureLoader& GraphicsContext::getTextureLoader()
{
    return *_textureLoader;
}

graphic::IFontLoader& GraphicsContext::getFontLoader()
{
    return *_fontLoader;
}

} // namespace zappy
