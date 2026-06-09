#include "HudInfoBehavior.hpp"
#include "graphic/IRenderer.hpp"

namespace behavior {

static constexpr int   FONT_SIZE   = 18;
static constexpr float LINE_HEIGHT = 22.f;

void HudInfoBehavior::onUpdate([[maybe_unused]] graphic::Entity& owner, float dt)
{
    _elapsed += dt;
}

void HudInfoBehavior::draw(graphic::IRenderer& renderer,
                           [[maybe_unused]] const graphic::Matrix4x4& transform)
{
    if (_lines.empty()) return;

    float panelW = 220.f;
    float panelH = _padding * 2.f + static_cast<float>(_lines.size()) * LINE_HEIGHT;

    graphic::ShapeStyle bgStyle;
    bgStyle.fill         = graphic::SolidFill{ {0, 0, 0, 160} };
    bgStyle.cornerRadius = 6.f;
    bgStyle.opacity      = 1.f;

    graphic::ShapeStyle borderStyle;
    borderStyle.stroke = graphic::Stroke{
        graphic::SolidFill{ {80, 80, 80, 200} },
        1.5f
    };
    borderStyle.cornerRadius = 6.f;

    graphic::Rectangle2D panel{ _pos, {panelW, panelH} };
    renderer.drawRect(panel, bgStyle);
    renderer.drawRect(panel, borderStyle);

    graphic::TextStyle textStyle;
    textStyle.font    = graphic::FontHandle{0};
    textStyle.size    = FONT_SIZE;
    textStyle.opacity = 1.f;

    float y = _pos.y + _padding;
    for (const auto& line : _lines) {
        textStyle.color = line.color;
        renderer.drawText(line.text, {_pos.x + _padding, y}, textStyle);
        y += LINE_HEIGHT;
    }
}

void HudInfoBehavior::setLines(std::vector<Line> lines)
{
    _lines = std::move(lines);
}

void HudInfoBehavior::setPosition(graphic::Vector2f pos)
{
    _pos = pos;
}

void HudInfoBehavior::setPadding(float padding)
{
    _padding = padding;
}

} // namespace behavior
