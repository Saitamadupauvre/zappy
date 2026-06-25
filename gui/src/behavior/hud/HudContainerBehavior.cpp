#include "HudContainerBehavior.hpp"
#include "behavior/drawable/ADrawable.hpp"
#include "behavior/rectTransform/RectTransformBehavior.hpp"
#include "behavior/hud/LayoutEngine.hpp"
#include "event/Event.hpp"
#include "util/Overloaded.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace behavior {

AnchorWeights HudContainerBehavior::getAnchorWeights(Anchor anchor)
{
    switch (anchor) {
        case Anchor::TopLeft:      return {0.0f, 0.0f};
        case Anchor::TopCenter:    return {0.5f, 0.0f};
        case Anchor::TopRight:     return {1.0f, 0.0f};
        case Anchor::MiddleLeft:   return {0.0f, 0.5f};
        case Anchor::Center:       return {0.5f, 0.5f};
        case Anchor::MiddleRight:  return {1.0f, 0.5f};
        case Anchor::BottomLeft:   return {0.0f, 1.0f};
        case Anchor::BottomCenter: return {0.5f, 1.0f};
        case Anchor::BottomRight:  return {1.0f, 1.0f};
        default:                   return {0.0f, 0.0f};
    }
}

graphic::Vector2f HudContainerBehavior::measureElement(graphic::IRenderer& renderer,
                                                        const hud::HudElement& el) const
{
    float s = _uiScale;
    return std::visit(overloaded{
        [&](const hud::TextData& arg) -> graphic::Vector2f {
            graphic::TextStyle style;
            style.font  = graphic::FontHandle{0};
            style.size  = arg.fontSize * s;
            style.color = arg.color;
            return renderer.measureText(arg.content, style);
        },
        [&](const hud::BarData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::RectData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::ButtonData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::ChatBubbleData& arg) -> graphic::Vector2f {
            graphic::TextStyle ts;
            ts.font  = graphic::FontHandle{0};
            ts.size  = arg.fontSize * s;
            ts.color = {255, 255, 255, 255};
            auto textSz   = renderer.measureText(arg.text,   ts);
            auto senderSz = renderer.measureText(arg.sender, ts);
            float w = std::min(std::max(textSz.x, senderSz.x) + 16.0f * s, arg.maxWidth * s);
            float h = arg.fontSize * s * 2.2f + 14.0f * s;
            return {w, h};
        },
        [&](const hud::ImageData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::SliderData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::SlotData& arg) -> graphic::Vector2f {
            return {arg.slotSize * s, arg.slotSize * s};
        },
        [&](const hud::SelectData& arg) -> graphic::Vector2f {
            float rows = arg.isOpen ? 1.f + static_cast<float>(arg.options.size()) : 1.f;
            return {arg.width * s, arg.rowHeight * rows * s};
        },
        [&](const hud::ToggleData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
        [&](const hud::ImageButtonData& arg) -> graphic::Vector2f {
            return {arg.width * s, arg.height * s};
        },
    }, el.data);
}

std::vector<graphic::Vector2f> HudContainerBehavior::measureAll(
    graphic::IRenderer& renderer, const std::vector<hud::HudElement>& elements) const
{
    std::vector<graphic::Vector2f> sizes;
    sizes.reserve(elements.size());
    for (auto& el : elements)
        sizes.push_back(measureElement(renderer, el));
    return sizes;
}

graphic::Vector2f HudContainerBehavior::contentSize(
    const std::vector<graphic::Vector2f>& sizes) const
{
    float maxW = 0.f, totalH = 0.f;
    for (auto& s : sizes) {
        maxW    = std::max(maxW, s.x);
        totalH += s.y + _padding;
    }
    return {maxW + _padding * 4.f, totalH + _padding};
}

void HudContainerBehavior::applyAnchor(graphic::IRenderer& renderer,
                                        behavior::RectTransformBehavior* rect)
{
    auto vp = renderer.getViewportSize();
    auto sz = rect->getSize();
    float margin = 20.0f * _uiScale;

    auto [wx, wy] = getAnchorWeights(_anchor);
    float x = wx * (vp.x - sz.x) + (wx == 0.f ? margin : wx == 1.f ? -margin : 0.f);
    float y = wy * (vp.y - sz.y) + (wy == 0.f ? margin : wy == 1.f ? -margin : 0.f);
    rect->setPosition({x + _anchorOffset.x * _uiScale, y + _anchorOffset.y * _uiScale});
}

void HudContainerBehavior::drawBackground(graphic::IRenderer& renderer,
                                           const graphic::Vector2f& pos,
                                           const graphic::Vector2f& sz)
{
    graphic::Rectangle2D panel{pos, sz};

    graphic::ShapeStyle bg;
    bg.fill         = graphic::Fill{graphic::SolidFill{_bgColor}};
    bg.cornerRadius = 6.0f;
    bg.opacity      = _animAlpha;
    renderer.drawRect(panel, bg);

    graphic::ShapeStyle border;
    border.stroke       = graphic::Stroke{graphic::SolidFill{_borderColor}, _borderWidth};
    border.cornerRadius = 6.0f;
    border.opacity      = _animAlpha;
    renderer.drawRect(panel, border);
}

void HudContainerBehavior::drawElement(graphic::IRenderer& renderer,
                                        const hud::HudElement& el,
                                        const graphic::Vector2f& pos,
                                        const graphic::Vector2f& size)
{
    float sc = _uiScale;
    std::visit(overloaded{
        [&](const hud::TextData& arg) {
            graphic::TextStyle style;
            style.color   = arg.color;
            style.size    = arg.fontSize * sc;
            style.opacity = _animAlpha;
            renderer.drawText(arg.content, pos, style);
        },
        [&](const hud::BarData& arg) {
            graphic::ShapeStyle s;
            s.fill    = graphic::Fill{graphic::SolidFill{arg.fillColor}};
            s.opacity = _animAlpha;
            renderer.drawRect({pos, {size.x * arg.ratio, size.y}}, s);
        },
        [&](const hud::RectData& arg) {
            graphic::ShapeStyle s;
            s.fill         = graphic::Fill{graphic::SolidFill{arg.fillColor}};
            s.stroke       = graphic::Stroke{graphic::SolidFill{arg.borderColor}, 2.0f};
            s.cornerRadius = 4.0f;
            s.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, s);
        },
        [&](const hud::ButtonData& arg) {
            bool hovered = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + size.x &&
                            _lastMousePos.y >= pos.y && _lastMousePos.y <= pos.y + size.y);

            graphic::ShapeStyle s;
            s.fill         = graphic::Fill{graphic::SolidFill{hovered ? arg.hoverBgColor : arg.bgColor}};
            s.cornerRadius = 6.0f;
            s.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, s);

            graphic::TextStyle ts;
            ts.size    = arg.fontSize * sc;
            ts.color   = arg.textColor;
            ts.opacity = _animAlpha;
            auto textSz = renderer.measureText(arg.label, ts);
            renderer.drawText(arg.label,
                {pos.x + (size.x - textSz.x) * 0.5f, pos.y + (size.y - textSz.y) * 0.5f},
                ts);

            _buttonAreas.push_back({pos, size, arg.onClick});
        },
        [&](const hud::ChatBubbleData& arg) {
            graphic::ShapeStyle bs;
            bs.fill         = graphic::Fill{graphic::SolidFill{arg.bubbleColor}};
            bs.cornerRadius = 8.0f;
            bs.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, bs);

            graphic::TextStyle senderStyle;
            senderStyle.size    = (arg.fontSize - 1.0f) * sc;
            senderStyle.color   = {210, 210, 210, 255};
            senderStyle.opacity = _animAlpha;
            renderer.drawText(arg.sender, {pos.x + 6.0f * sc, pos.y + 4.0f * sc}, senderStyle);

            graphic::TextStyle msgStyle;
            msgStyle.size    = arg.fontSize * sc;
            msgStyle.color   = {255, 255, 255, 255};
            msgStyle.opacity = _animAlpha;
            renderer.drawText(arg.text, {pos.x + 6.0f * sc, pos.y + arg.fontSize * sc + 8.0f * sc}, msgStyle);
        },
        [&](const hud::ImageData& arg) {
            graphic::SpriteDrawParams sp;
            sp.texture = arg.texture;
            sp.srcRect = {{0.f, 0.f}, {1.f, 1.f}};
            sp.dstRect = {pos, size};
            sp.tint    = arg.tint;
            sp.opacity = _animAlpha * arg.opacity;
            renderer.drawSprite(sp);
        },
        [&](const hud::SliderData& arg) {
            float ratio = std::clamp((arg.value - arg.min) / (arg.max - arg.min), 0.f, 1.f);

            graphic::ShapeStyle track;
            track.fill         = graphic::Fill{graphic::SolidFill{{55, 55, 75, 200}}};
            track.cornerRadius = size.y * 0.5f;
            track.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, track);

            if (ratio > 0.f) {
                graphic::ShapeStyle fill;
                fill.fill         = graphic::Fill{graphic::SolidFill{{40, 110, 210, 220}}};
                fill.cornerRadius = size.y * 0.5f;
                fill.opacity      = _animAlpha;
                renderer.drawRect({pos, {size.x * ratio, size.y}}, fill);
            }

            float hw = 10.f * sc;
            float hh = size.y + 4.f * sc;
            float hx = pos.x + size.x * ratio - hw * 0.5f;
            float hy = pos.y - 2.f * sc;
            graphic::ShapeStyle handle;
            handle.fill         = graphic::Fill{graphic::SolidFill{{210, 220, 255, 255}}};
            handle.cornerRadius = 3.f * sc;
            handle.opacity      = _animAlpha;
            renderer.drawRect({{hx, hy}, {hw, hh}}, handle);

            _sliderAreas.push_back({pos, size, arg.min, arg.max, arg.onChange, arg.onRelease});
        },
        [&](const hud::SelectData& arg) {
            float rh = arg.rowHeight * sc;

            // header button
            bool hovered = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + size.x &&
                            _lastMousePos.y >= pos.y && _lastMousePos.y <= pos.y + rh);
            auto headerBg = hovered ? arg.headerHover : arg.headerBg;

            graphic::ShapeStyle hs;
            hs.fill         = graphic::Fill{graphic::SolidFill{headerBg}};
            hs.cornerRadius = arg.isOpen ? 6.f : 6.f;
            hs.opacity      = _animAlpha;
            renderer.drawRect({pos, {size.x, rh}}, hs);

            std::string headerLabel = arg.label + ":  " +
                (arg.currentIndex >= 0 && arg.currentIndex < static_cast<int>(arg.options.size())
                    ? arg.options[arg.currentIndex] : "?") +
                (arg.isOpen ? "  ^" : "  v");

            graphic::TextStyle hts;
            hts.size    = arg.fontSize * sc;
            hts.color   = arg.textColor;
            hts.opacity = _animAlpha;
            auto hTextSz = renderer.measureText(headerLabel, hts);
            renderer.drawText(headerLabel,
                {pos.x + (size.x - hTextSz.x) * 0.5f, pos.y + (rh - hTextSz.y) * 0.5f}, hts);

            _buttonAreas.push_back({pos, {size.x, rh}, arg.onToggle});

            // option rows (only when open)
            if (arg.isOpen) {
                for (int i = 0; i < static_cast<int>(arg.options.size()); ++i) {
                    float oy = pos.y + rh * (1 + i);
                    bool optHovered = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + size.x &&
                                       _lastMousePos.y >= oy && _lastMousePos.y <= oy + rh);
                    bool selected   = (i == arg.currentIndex);

                    auto bg = selected ? arg.selectedBg : (optHovered ? arg.optionHover : arg.optionBg);
                    graphic::ShapeStyle os;
                    os.fill         = graphic::Fill{graphic::SolidFill{bg}};
                    os.cornerRadius = (i == static_cast<int>(arg.options.size()) - 1) ? 6.f : 0.f;
                    os.opacity      = _animAlpha;
                    renderer.drawRect({{pos.x, oy}, {size.x, rh}}, os);

                    graphic::TextStyle ots;
                    ots.size    = arg.fontSize * sc;
                    ots.color   = arg.textColor;
                    ots.opacity = _animAlpha;
                    auto oTextSz = renderer.measureText(arg.options[i], ots);
                    renderer.drawText(arg.options[i],
                        {pos.x + (size.x - oTextSz.x) * 0.5f, oy + (rh - oTextSz.y) * 0.5f}, ots);

                    int idx = i;
                    _buttonAreas.push_back({{pos.x, oy}, {size.x, rh},
                        [onSelect = arg.onSelect, idx]() { if (onSelect) onSelect(idx); }});
                }
            }
        },
        [&](const hud::SlotData& arg) {
            float sc = _uiScale;

            graphic::ShapeStyle bg;
            bg.fill         = graphic::Fill{graphic::SolidFill{arg.bgColor}};
            bg.cornerRadius = 3.f;
            bg.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, bg);

            graphic::ShapeStyle border;
            border.stroke       = graphic::Stroke{graphic::SolidFill{arg.borderColor}, 2.f};
            border.cornerRadius = 3.f;
            border.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, border);

            float imgSz = arg.imageSize * sc;
            float imgX  = pos.x + (size.x - imgSz) * 0.5f;
            float imgY  = pos.y + (size.y - imgSz) * 0.5f - arg.fontSize * sc * 0.4f;
            if (arg.texture.id != 0) {
                graphic::SpriteDrawParams sp;
                sp.texture = arg.texture;
                sp.srcRect = {{0.f, 0.f}, {1.f, 1.f}};
                sp.dstRect = {{imgX, imgY}, {imgSz, imgSz}};
                sp.tint    = {255, 255, 255, 255};
                sp.opacity = _animAlpha;
                renderer.drawSprite(sp);
            }

            if (!arg.label.empty()) {
                graphic::TextStyle ts;
                ts.size    = arg.fontSize * sc;
                ts.color   = arg.textColor;
                ts.opacity = _animAlpha;
                auto textSz = renderer.measureText(arg.label, ts);
                renderer.drawText(arg.label,
                    {pos.x + size.x - textSz.x - 3.f * sc,
                     pos.y + size.y - textSz.y - 2.f * sc},
                    ts);
            }
        },
        [&](const hud::ToggleData& arg) {
            bool hovered = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + size.x &&
                            _lastMousePos.y >= pos.y && _lastMousePos.y <= pos.y + size.y);
            auto bgColor = arg.value
                ? (hovered ? arg.hoverOn  : arg.bgOn)
                : (hovered ? arg.hoverOff : arg.bgOff);

            graphic::ShapeStyle bg;
            bg.fill         = graphic::Fill{graphic::SolidFill{bgColor}};
            bg.cornerRadius = size.y * 0.4f;
            bg.opacity      = _animAlpha;
            renderer.drawRect({pos, size}, bg);

            // label on left
            graphic::TextStyle lts;
            lts.size    = arg.fontSize * sc;
            lts.color   = arg.textColor;
            lts.opacity = _animAlpha;
            auto labelSz = renderer.measureText(arg.label, lts);
            renderer.drawText(arg.label,
                {pos.x + 12.f * sc, pos.y + (size.y - labelSz.y) * 0.5f}, lts);

            // ON / OFF badge on right
            const std::string badge = arg.value ? "ON" : "OFF";
            float badgeW = 38.f * sc;
            float badgeH = size.y - 8.f * sc;
            float badgeX = pos.x + size.x - badgeW - 6.f * sc;
            float badgeY = pos.y + 4.f * sc;
            graphic::Color4b badgeBg = arg.value
                ? graphic::Color4b{50, 200, 100, 230}
                : graphic::Color4b{180,  60,  60, 200};
            graphic::ShapeStyle bs;
            bs.fill         = graphic::Fill{graphic::SolidFill{badgeBg}};
            bs.cornerRadius = badgeH * 0.4f;
            bs.opacity      = _animAlpha;
            renderer.drawRect({{badgeX, badgeY}, {badgeW, badgeH}}, bs);

            graphic::TextStyle bts;
            bts.size    = (arg.fontSize - 1.f) * sc;
            bts.color   = {255, 255, 255, 255};
            bts.opacity = _animAlpha;
            auto badgeSz = renderer.measureText(badge, bts);
            renderer.drawText(badge,
                {badgeX + (badgeW - badgeSz.x) * 0.5f, badgeY + (badgeH - badgeSz.y) * 0.5f},
                bts);

            _buttonAreas.push_back({pos, size,
                [onToggle = arg.onToggle, val = arg.value]() {
                    if (onToggle) onToggle(!val);
                }});
        },
        [&](const hud::ImageButtonData& arg) {
            bool hovered = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + size.x &&
                            _lastMousePos.y >= pos.y && _lastMousePos.y <= pos.y + size.y);

            graphic::SpriteDrawParams sp;
            sp.texture = arg.texture;
            sp.srcRect = {{0.f, 0.f}, {1.f, 1.f}};
            sp.dstRect = {pos, size};
            sp.tint    = hovered ? arg.hoverTint : arg.tint;
            sp.opacity = _animAlpha * arg.opacity;
            renderer.drawSprite(sp);

            _buttonAreas.push_back({pos, size, arg.onClick});
        },
    }, el.data);
}

void HudContainerBehavior::setVisible(bool visible, float duration)
{
    float dur     = (duration >= 0.f) ? duration : _animDuration;
    bool  instant = !_animEnabled || dur <= 0.f;
    _isVisible    = visible;

    if (visible) {
        if (_animState == AnimState::Visible) return;
        if (instant) { _animState = AnimState::Visible; _animAlpha = 1.f; _animSlideOffset = {0.f,0.f}; return; }
        if (_animState != AnimState::FadingIn) {
            bool wasHidden = (_animState == AnimState::Hidden);
            _animElapsed = (_animState == AnimState::FadingOut)
                           ? (1.f - std::clamp(_animElapsed / _animDuration, 0.f, 1.f)) * dur
                           : 0.f;
            _animDuration   = dur;
            _animState      = AnimState::FadingIn;
            if (wasHidden) _lastFrameValid = false;
        }
    } else {
        if (_animState == AnimState::Hidden) return;
        if (instant) { _animState = AnimState::Hidden; _animAlpha = 0.f; _animSlideOffset = {0.f,0.f}; return; }
        if (_animState != AnimState::FadingOut) {
            _animElapsed = (_animState == AnimState::FadingIn)
                           ? (1.f - std::clamp(_animElapsed / _animDuration, 0.f, 1.f)) * dur
                           : 0.f;
            _animDuration = dur;
            _animState    = AnimState::FadingOut;
        }
    }
}

static float smoothstep(float t) { return t * t * (3.f - 2.f * t); }

graphic::Vector2f HudContainerBehavior::slideDirection() const
{
    switch (_anchor) {
        case Anchor::TopLeft:
        case Anchor::MiddleLeft:
        case Anchor::BottomLeft:   return {-1.f,  0.f};
        case Anchor::TopRight:
        case Anchor::MiddleRight:
        case Anchor::BottomRight:  return { 1.f,  0.f};
        case Anchor::TopCenter:    return { 0.f, -1.f};
        case Anchor::BottomCenter: return { 0.f,  1.f};
        default:                   return { 0.f, -1.f};
    }
}

void HudContainerBehavior::tickAnimation(float dt)
{
    if (_animState == AnimState::Visible || _animState == AnimState::Hidden) return;
    _animElapsed += dt;
    float t   = std::clamp(_animElapsed / _animDuration, 0.f, 1.f);
    float te  = smoothstep(t);
    auto  dir = slideDirection();
    float dist = (std::abs(dir.x) > 0.5f ? _lastSize.x : _lastSize.y) + 30.f;

    if (_animState == AnimState::FadingIn) {
        _animAlpha       = te;
        _animSlideOffset = dir * dist * (1.f - te);
        if (t >= 1.f) { _animState = AnimState::Visible; _animAlpha = 1.f; _animSlideOffset = {0.f, 0.f}; }
    } else {
        _animAlpha       = 1.f - te;
        _animSlideOffset = dir * dist * te;
        if (t >= 1.f) { _animState = AnimState::Hidden;  _animAlpha = 0.f; _animSlideOffset = {0.f, 0.f}; }
    }
}

void HudContainerBehavior::onEvent(graphic::Entity& owner, const event::Event& ev)
{
    ADrawable::onEvent(owner, ev);

    auto fireSlider = [&](int idx, float mouseX, bool release) {
        if (idx < 0 || idx >= static_cast<int>(_sliderAreas.size())) return;
        auto& sl = _sliderAreas[idx];
        float ratio = std::clamp((mouseX - sl.pos.x) / sl.size.x, 0.f, 1.f);
        float val   = sl.min + ratio * (sl.max - sl.min);
        if (sl.onChange)  sl.onChange(val);
        if (release && sl.onRelease) sl.onRelease(val);
    };

    event::on(ev,
        [&](const event::MouseMoveEvent& e) {
            _lastMousePos = e.position;
            if (_draggingSlider >= 0)
                fireSlider(_draggingSlider, e.position.x, false);
        },
        [&](const event::MouseWheelEvent& e) {
            if (!_isScrollable) return;
            auto rect = _owner.getBehavior<behavior::RectTransformBehavior>();
            if (!rect) return;
            auto pos = rect->getPosition();
            auto sz  = rect->getSize();
            bool over = (_lastMousePos.x >= pos.x && _lastMousePos.x <= pos.x + sz.x &&
                         _lastMousePos.y >= pos.y && _lastMousePos.y <= pos.y + sz.y);
            if (!over) return;
            _scrollOffsetY -= e.delta * 30.0f * _uiScale;
            float visibleH  = sz.y - (!_title.empty() ? TITLE_BAR_HEIGHT : 0.0f);
            float maxScroll = std::max(0.0f, _contentHeight - visibleH);
            _scrollOffsetY  = std::clamp(_scrollOffsetY, 0.0f, maxScroll);
        },
        [&](const event::MouseButtonEvent& e) {
            if (e.button != graphic::MouseBtn::LEFT) return;
            if (!e.pressed) {
                if (_draggingSlider >= 0)
                    fireSlider(_draggingSlider, e.screenPos.x, true);
                _draggingSlider = -1;
                return;
            }

            for (int i = 0; i < static_cast<int>(_sliderAreas.size()); ++i) {
                auto& sl = _sliderAreas[i];
                bool hit = (e.screenPos.x >= sl.pos.x && e.screenPos.x <= sl.pos.x + sl.size.x &&
                            e.screenPos.y >= sl.pos.y && e.screenPos.y <= sl.pos.y + sl.size.y);
                if (hit) {
                    _draggingSlider = i;
                    fireSlider(i, e.screenPos.x, false);
                    return;
                }
            }
            _draggingSlider = -1;

            for (auto& btn : _buttonAreas) {
                bool hit = (e.screenPos.x >= btn.pos.x && e.screenPos.x <= btn.pos.x + btn.size.x &&
                            e.screenPos.y >= btn.pos.y && e.screenPos.y <= btn.pos.y + btn.size.y);
                if (hit && btn.onClick) {
                    btn.onClick();
                    break;
                }
            }
        }
    );
}

void HudContainerBehavior::draw(graphic::IRenderer& renderer,
                                 [[maybe_unused]] const graphic::Matrix4x4& transform)
{
    auto now = std::chrono::steady_clock::now();
    float dt = 0.f;
    if (_lastFrameValid)
        dt = std::clamp(std::chrono::duration<float>(now - _lastFrameTime).count(), 0.f, 0.1f);
    _lastFrameTime  = now;
    _lastFrameValid = true;
    tickAnimation(dt);

    if (!_provider || _animState == AnimState::Hidden) {
        _buttonAreas.clear();
        _sliderAreas.clear();
        return;
    }

    {
        auto vp = renderer.getViewportSize();
        _uiScale = std::max(0.5f, vp.x / 1280.0f);
    }

    auto rect = _owner.getBehavior<behavior::RectTransformBehavior>();
    if (!rect) return;

    uint64_t currentVersion = _provider->getVersion();
    bool scaleChanged = (_uiScale != _lastUiScale);
    bool needRebuild = (currentVersion == 0)
                    || (currentVersion != _lastProviderVersion)
                    || _cachedElements.empty()
                    || scaleChanged;
    if (needRebuild) {
        _cachedElements      = _provider->getHudElements();
        _cachedSizes         = measureAll(renderer, _cachedElements);
        if (currentVersion != 0)
            _lastProviderVersion = currentVersion;
        _lastUiScale = _uiScale;
    }
    const auto& elements = _cachedElements;
    const auto& sizes    = _cachedSizes;

    if (elements.empty()) return;

    float scaledPadding = _padding * _uiScale;
    graphic::Vector2f sz = (_sizeMode == graphic::SizeMode::Auto)
                           ? [&]() {
                               float maxW = 0.f, totalH = 0.f;
                               for (auto& s : sizes) {
                                   maxW    = std::max(maxW, s.x);
                                   totalH += s.y + scaledPadding;
                               }
                               return graphic::Vector2f{maxW + scaledPadding * 4.f, totalH + scaledPadding};
                           }()
                           : graphic::Vector2f{_fixedSize.x * _uiScale, _fixedSize.y * _uiScale};
    rect->setSize(sz);

    _lastSize = sz;

    if (!_isWorldSpaceTag)
        applyAnchor(renderer, rect.get());

    if (!_isWorldSpaceTag && _animState != AnimState::Visible) {
        auto p = rect->getPosition();
        rect->setPosition({p.x + _animSlideOffset.x, p.y + _animSlideOffset.y});
    }

    auto layouts = hud::LayoutEngine::calculate(_layoutType, sizes, scaledPadding, sz, _groupStride);

    {
        float h = 0.f;
        for (auto& l : layouts) h = std::max(h, l.position.y + l.size.y);
        _contentHeight = h + scaledPadding;
    }
    if (_isScrollable) {
        float visibleH  = sz.y - (!_title.empty() ? TITLE_BAR_HEIGHT * _uiScale : 0.0f);
        float maxScroll = std::max(0.0f, _contentHeight - visibleH);
        _scrollOffsetY  = std::clamp(_scrollOffsetY, 0.0f, maxScroll);
    }

    if (_hasBackground)
        drawBackground(renderer, rect->getPosition(), sz);

    graphic::Vector2f origin = rect->getPosition();

    if (!_title.empty()) {
        float barH = TITLE_BAR_HEIGHT * _uiScale;

        graphic::ShapeStyle barStyle;
        barStyle.fill         = graphic::Fill{graphic::SolidFill{{25, 25, 55, 240}}};
        barStyle.cornerRadius = 5.0f;
        barStyle.opacity      = _animAlpha;
        renderer.drawRect({origin, {sz.x, barH}}, barStyle);

        graphic::TextStyle ts;
        ts.size    = _titleFontSize * _uiScale;
        ts.color   = {200, 210, 255, 255};
        ts.opacity = _animAlpha;
        auto textSz = renderer.measureText(_title, ts);
        renderer.drawText(_title,
            {origin.x + (sz.x - textSz.x) * 0.5f,
             origin.y + (barH - textSz.y) * 0.5f},
            ts);

        origin.y += barH;
        sz.y     -= barH;
    }

    _buttonAreas.clear();
    _sliderAreas.clear();

    if (_isScrollable)
        renderer.beginScissor(
            static_cast<int>(origin.x),
            static_cast<int>(origin.y),
            static_cast<int>(sz.x),
            static_cast<int>(sz.y));

    for (size_t i = 0; i < elements.size(); ++i) {
        float rawY  = (origin + layouts[i].position).y;
        float drawY = _isScrollable ? rawY - _scrollOffsetY : rawY;

        if (_isScrollable) {
            if (drawY + sizes[i].y <= origin.y) continue;
            if (drawY >= origin.y + sz.y)       continue;
        }

        graphic::Vector2f drawPos;
        if (auto* bubble = std::get_if<hud::ChatBubbleData>(&elements[i].data)) {
            float bw = sizes[i].x;
            float bx = bubble->isLeft
                ? origin.x + scaledPadding
                : origin.x + sz.x - bw - scaledPadding;
            drawPos = {bx, drawY};
        } else {
            drawPos = {(origin + layouts[i].position).x, drawY};
        }

        drawElement(renderer, elements[i], drawPos, sizes[i]);
    }

    if (_isScrollable)
        renderer.endScissor();
}

} // namespace behavior
