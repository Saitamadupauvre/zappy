#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include "graphic/Types.hpp"

namespace behavior::hud {

    struct RectData {
        float width;
        float height;
        graphic::Color4b borderColor;
        graphic::Color4b fillColor;
    };

    struct TextData {
        std::string content;
        float fontSize = 16.0f;
        graphic::Color4b color = graphic::Color4b::white();
    };

    struct BarData {
        float ratio;
        graphic::Color4b fillColor = graphic::Color4b::red();
        float width = 100.0f;
        float height = 10.0f;
    };

    struct ButtonData {
        std::string label;
        float fontSize = 14.0f;
        float width = 130.0f;
        float height = 28.0f;
        graphic::Color4b textColor    = {255, 255, 255, 255};
        graphic::Color4b bgColor      = {40,  110, 210, 220};
        graphic::Color4b hoverBgColor = {70,  145, 255, 230};
        std::function<void()> onClick;
    };

    struct ChatBubbleData {
        std::string text;
        std::string sender;
        bool isLeft = true;
        graphic::Color4b bubbleColor = {40, 80, 180, 220};
        float maxWidth = 240.0f;
        float fontSize = 13.0f;
    };

    struct ImageData {
        graphic::TextureHandle texture;
        float width  = 64.0f;
        float height = 64.0f;
        float opacity = 1.0f;
        graphic::Color4b tint = graphic::Color4b::white();
    };

    struct SliderData {
        float min    = 1.f;
        float max    = 100.f;
        float value  = 1.f;
        float width  = 200.f;
        float height = 14.f;
        std::function<void(float)> onChange;   // fires during drag (visual feedback)
        std::function<void(float)> onRelease;  // fires on mouse release (commit)
    };

    struct SlotData {
        graphic::TextureHandle texture;
        std::string label;
        float slotSize   = 52.f;
        float imageSize  = 36.f;
        float fontSize   = 10.f;
        graphic::Color4b bgColor      = {25, 25, 25, 230};
        graphic::Color4b borderColor  = {90, 90, 90, 255};
        graphic::Color4b textColor    = {255, 255, 255, 255};
    };

    struct ToggleData {
        std::string      label;
        bool             value       = false;
        float            width       = 260.f;
        float            height      = 32.f;
        float            fontSize    = 12.f;
        graphic::Color4b textColor   = {255, 255, 255, 255};
        graphic::Color4b bgOn        = {30,  120,  60, 220};
        graphic::Color4b bgOff       = {100,  35,  35, 210};
        graphic::Color4b hoverOn     = {45,  160,  80, 235};
        graphic::Color4b hoverOff    = {140,  50,  50, 230};
        std::function<void(bool)> onToggle;
    };

    struct SelectData {
        std::string              label;
        std::vector<std::string> options;
        int                      currentIndex = 0;
        bool                     isOpen       = false;
        float                    width        = 260.f;
        float                    rowHeight    = 28.f;
        float                    fontSize     = 12.f;
        graphic::Color4b textColor    = {255, 255, 255, 255};
        graphic::Color4b headerBg     = {40,  50,  110, 210};
        graphic::Color4b headerHover  = {60,  75,  150, 230};
        graphic::Color4b optionBg     = {25,  30,  70,  220};
        graphic::Color4b optionHover  = {45,  60,  120, 235};
        graphic::Color4b selectedBg   = {35,  85,  155, 235};
        std::function<void()>    onToggle;
        std::function<void(int)> onSelect;
    };

    struct ImageButtonData {
        graphic::TextureHandle texture;
        float width  = 64.0f;
        float height = 64.0f;
        graphic::Color4b tint      = graphic::Color4b::white();
        graphic::Color4b hoverTint = {200, 200, 255, 255};
        float opacity = 1.0f;
        std::function<void()> onClick;
    };

    using HudElementData = std::variant<TextData, BarData, RectData, ButtonData, ChatBubbleData, ImageData, SliderData, SlotData, SelectData, ToggleData, ImageButtonData>;

    struct HudElement {
        HudElementData data;
    };

} // namespace behavior::hud