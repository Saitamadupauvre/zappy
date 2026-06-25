#pragma once

#include "graphic/IWindowContext.hpp"
#include <raylib.h>

namespace graphic::raylib {

class RaylibWindow : public IWindowContext
{
public:
    RaylibWindow();
    ~RaylibWindow() override;

    void     create(int width, int height, const std::string& title, int targetFps = 60) override;
    void     setTargetFps(int fps) override;
    void     setFullscreen(bool enable) override;
    void     setResolution(int width, int height) override;
    bool     isOpen()       const override;
    void     close()              override;
    float    getDeltaTime() const override;
    Vector2f getSize()      const override;
    void     beginFrame()         override;
    void     endFrame()           override;

    std::vector<event::Event> pollEvents() override;

private:
    bool     _closed = false;
    Vector2f _lastSize{0.f, 0.f};
};

} // namespace graphic::raylib
