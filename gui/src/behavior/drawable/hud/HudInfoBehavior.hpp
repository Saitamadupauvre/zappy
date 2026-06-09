#pragma once

#include "behavior/drawable/ADrawable.hpp"
#include "graphic/Types.hpp"
#include <string>
#include <vector>

namespace behavior {

class HudInfoBehavior : public ADrawable
{
    public:
        struct Line {
            std::string     text;
            graphic::Color4b color = graphic::Color4b::white();
        };

        void onUpdate(graphic::Entity&, float dt) override;
        void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

        void setLines(std::vector<Line> lines);
        void setPosition(graphic::Vector2f pos);
        void setPadding(float padding);

    private:
        std::vector<Line>   _lines;
        graphic::Vector2f   _pos     = {10.f, 10.f};
        float               _padding = 8.f;
        float               _elapsed = 0.f;
};

} // namespace behavior
