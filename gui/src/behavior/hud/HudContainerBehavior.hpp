#pragma once

#include "entity/Entity.hpp"
#include "behavior/rectTransform/RectTransformBehavior.hpp"
#include "behavior/drawable/ADrawable.hpp"
#include "hud/HudElements.hpp"
#include "hud/IHudProvider.hpp"
#include "graphic/Types.hpp"
#include "LayoutEngine.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

using Anchor = graphic::Anchor;
using AnchorWeights = graphic::AnchorWeights;

namespace behavior {

class HudContainerBehavior : public ADrawable {
public:
    explicit HudContainerBehavior(graphic::Entity& owner) : _owner(owner) {}
    ~HudContainerBehavior() = default;

    void setProvider(std::shared_ptr<hud::IHudProvider> provider) { _provider = provider; }
    void setLayout(hud::LayoutEngine::Type type, float padding, int groupStride = 0) {
        _layoutType  = type;
        _padding     = padding;
        _groupStride = groupStride;
    }
    void setAnchor(graphic::Anchor anchor) { _anchor = anchor; }
    void setBackground(bool enabled, graphic::Color4b fillColor, graphic::Color4b borderColor) {
        _hasBackground = enabled;
        _bgColor       = fillColor;
        _borderColor   = borderColor;
    }
    void setBorderWidth(float w) { _borderWidth = w; }
    void setWorldSpaceTag(bool isWorldSpaceTag) { _isWorldSpaceTag = isWorldSpaceTag; }
    void setVisible(bool visible, float duration = -1.f);
    void setSizeMode(graphic::SizeMode mode)    { _sizeMode = mode; }
    void setFixedSize(const graphic::Vector2f& size) { _fixedSize = size; }
    void setScrollable(bool scrollable)         { _isScrollable = scrollable; }
    void scrollToBottom()                       { _scrollOffsetY = 1e9f; }
    void scrollToTop()                          { _scrollOffsetY = 0.f; }
    void setAnchorOffset(const graphic::Vector2f& offset) { _anchorOffset = offset; }
    void setTitle(const std::string& title, float fontSize = 14.0f) {
        _title = title;
        _titleFontSize = fontSize;
    }

    bool isVisible()       const { return _isVisible; }
    bool isFullyVisible()  const { return _animState == AnimState::Visible; }
    bool isInteractable()  const { return _animState == AnimState::Visible || _animState == AnimState::FadingIn; }
    bool isScrollable()    const { return _isScrollable; }
    void setAnimationDuration(float s) { _animDuration = s; }
    void setAnimationEnabled(bool e)   { _animEnabled  = e; }

    static AnchorWeights getAnchorWeights(Anchor anchor);
    void onUpdate([[maybe_unused]] graphic::Entity& owner,
                  [[maybe_unused]] float deltaTime) override {}

    void onEvent(graphic::Entity& owner, const event::Event& ev) override;
    void draw(graphic::IRenderer& renderer, const graphic::Matrix4x4& transform) override;

private:
    struct ButtonArea {
        graphic::Vector2f     pos;
        graphic::Vector2f     size;
        std::function<void()> onClick;
    };

    struct SliderArea {
        graphic::Vector2f          pos;
        graphic::Vector2f          size;
        float                      min;
        float                      max;
        std::function<void(float)> onChange;
        std::function<void(float)> onRelease;
    };

    std::shared_ptr<hud::IHudProvider> _provider;
    graphic::Entity& _owner;

    mutable uint64_t                          _lastProviderVersion = UINT64_MAX;
    mutable std::vector<hud::HudElement>      _cachedElements;
    mutable std::vector<graphic::Vector2f>    _cachedSizes;

    graphic::Vector2f measureElement(graphic::IRenderer& renderer,
                                     const hud::HudElement& el) const;
    std::vector<graphic::Vector2f> measureAll(graphic::IRenderer& renderer,
                                              const std::vector<hud::HudElement>& elements) const;
    graphic::Vector2f contentSize(const std::vector<graphic::Vector2f>& sizes) const;

    void applyAnchor(graphic::IRenderer& renderer, behavior::RectTransformBehavior* rect);
    void drawBackground(graphic::IRenderer& renderer,
                        const graphic::Vector2f& pos, const graphic::Vector2f& sz);
    void drawElement(graphic::IRenderer& renderer, const hud::HudElement& el,
                     const graphic::Vector2f& pos, const graphic::Vector2f& size);

    graphic::Vector2f slideDirection() const;
    void tickAnimation(float dt);

    hud::LayoutEngine::Type _layoutType = hud::LayoutEngine::Type::Vertical;
    float _padding     = 8.0f;
    int   _groupStride = 0;

    bool _hasBackground = false;
    graphic::Color4b _bgColor    = {0, 0, 0, 0};
    graphic::Color4b _borderColor = {0, 0, 0, 0};
    float            _borderWidth = 1.5f;

    Anchor _anchor = Anchor::BottomRight;

    bool _isVisible       = true;
    bool _isWorldSpaceTag = false;
    bool _isScrollable    = false;

    enum class AnimState { Hidden, FadingIn, Visible, FadingOut };
    AnimState         _animState        = AnimState::Visible;
    float             _animElapsed      = 0.f;
    float             _animDuration     = 0.25f;
    bool              _animEnabled      = true;
    float             _animAlpha        = 1.0f;
    graphic::Vector2f _animSlideOffset  = {0.f, 0.f};
    graphic::Vector2f _lastSize         = {0.f, 0.f};
    std::chrono::steady_clock::time_point _lastFrameTime;
    bool              _lastFrameValid   = false;

    graphic::SizeMode _sizeMode    = graphic::SizeMode::Auto;
    graphic::Vector2f _fixedSize   = {0, 0};
    graphic::Vector2f _anchorOffset = {0, 0};

    float _scrollOffsetY = 0.0f;
    float _contentHeight = 0.0f;

    graphic::Vector2f    _lastMousePos;
    std::vector<ButtonArea>  _buttonAreas;
    std::vector<SliderArea>  _sliderAreas;
    int  _draggingSlider = -1;

    std::string _title;
    float       _titleFontSize = 14.0f;
    static constexpr float TITLE_BAR_HEIGHT = 34.0f;

    mutable float _uiScale     = 1.0f;
    mutable float _lastUiScale = -1.0f;
};

} // namespace behavior