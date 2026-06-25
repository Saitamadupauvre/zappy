#pragma once

#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include <vector>

namespace behavior::hud {

    struct ElementLayout {
        graphic::Vector2f position;
        graphic::Vector2f size;
    };

    class LayoutEngine {
    public:
        enum class Type { Vertical, Horizontal, MediaObject, VerticalMedia, Grid, MediaObjectHButtons };

        static std::vector<ElementLayout> calculate(Type type,
                                                    const std::vector<graphic::Vector2f>& sizes,
                                                    float padding,
                                                    graphic::Vector2f containerSize,
                                                    int groupStride = 0);

    private:
        static std::vector<ElementLayout> vertical(const std::vector<graphic::Vector2f>& sizes,
                                                   float padding, graphic::Vector2f containerSize);
        static std::vector<ElementLayout> horizontal(const std::vector<graphic::Vector2f>& sizes,
                                                     float padding, graphic::Vector2f containerSize);
        static std::vector<ElementLayout> mediaObject(const std::vector<graphic::Vector2f>& sizes,
                                                      float padding, graphic::Vector2f containerSize);
        static std::vector<ElementLayout> verticalMedia(const std::vector<graphic::Vector2f>& sizes,
                                                        float padding, graphic::Vector2f containerSize,
                                                        int groupStride);
        static std::vector<ElementLayout> grid(const std::vector<graphic::Vector2f>& sizes,
                                               float padding, graphic::Vector2f containerSize,
                                               int columns);
        static std::vector<ElementLayout> mediaObjectHButtons(const std::vector<graphic::Vector2f>& sizes,
                                                              float padding, graphic::Vector2f containerSize,
                                                              int buttonCount);
    };

}
