#pragma once
#include "event/Event.hpp"
#include "graphic/Types.hpp"
#include <string>
#include <vector>

namespace graphic {

class IWindowContext {
    public:
        virtual ~IWindowContext() = default;

        virtual void     create(int width, int height, const std::string& title, int targetFps = 60) = 0;
        virtual bool     isOpen()       const = 0;
        virtual void     close()              = 0;
        virtual float    getDeltaTime() const = 0;
        virtual Vector2f getSize()      const = 0;
        virtual void     beginFrame()         = 0;
        virtual void     endFrame()           = 0;

        virtual std::vector<event::Event> pollEvents() = 0;
};

} // namespace graphic
