#include "HudBuilder.hpp"
#include "behavior/rectTransform/RectTransformBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "behavior/selectable/SelectableBehavior.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/drawable/hud/InfoHudBehavior.hpp"

HudBuilder::HudBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& HudBuilder::rect(const graphic::Vector2f& pos, const graphic::Vector2f& size)
{
    auto rect = _entity->getBehavior<behavior::RectTransformBehavior>();
    if (!rect)
        rect = _entity->addBehavior<behavior::RectTransformBehavior>();
        
    rect->setPosition(pos);
    rect->setSize(size);
    return _owner;
}

EntityBuilder& HudBuilder::container(std::shared_ptr<behavior::hud::IHudProvider> provider)
{
    if (!_entity->getBehavior<behavior::RectTransformBehavior>())
        _entity->addBehavior<behavior::RectTransformBehavior>();

    auto hud = _entity->addBehavior<behavior::HudContainerBehavior>(*_entity);
    hud->setProvider(provider);
    
    return _owner;
}

EntityBuilder& HudBuilder::layout(behavior::hud::LayoutEngine::Type type, float padding, int groupStride)
{
    if (auto hud = _entity->getBehavior<behavior::HudContainerBehavior>()) {
        hud->setLayout(type, padding, groupStride);
    }
    return _owner;
}

EntityBuilder& HudBuilder::anchor(graphic::Anchor anchor) 
{
    if (auto hud = _entity->getBehavior<behavior::HudContainerBehavior>()) {
        hud->setAnchor(anchor);
    }
    return _owner;
}

EntityBuilder& HudBuilder::background(bool enabled, graphic::Color4b fill, graphic::Color4b border)
{
    if (auto hud = _entity->getBehavior<behavior::HudContainerBehavior>()) {
        hud->setBackground(enabled, fill, border);
    }
    return _owner;
}

EntityBuilder& HudBuilder::infoHud(std::shared_ptr<ResourceInfoProvider> provider)
{
    _entity->addBehavior<behavior::InfoHudBehavior>(provider);
    
    return _owner;
}

EntityBuilder& HudBuilder::isWorldSpaceTag(bool isWorldSpace)
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud) {
        hud->setWorldSpaceTag(isWorldSpace);
    }
    return _owner;
}

EntityBuilder& HudBuilder::boxSize(const graphic::Vector2f& size)
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud) {
        hud->setSizeMode(graphic::SizeMode::Fixed);
        hud->setFixedSize(size);
    }
    return _owner;
}

EntityBuilder& HudBuilder::autoSize()
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud) {
        hud->setSizeMode(graphic::SizeMode::Auto);
    }
    return _owner;
}

EntityBuilder& HudBuilder::fullscreen()
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud) {
        hud->setSizeMode(graphic::SizeMode::Fixed);
        hud->setFullscreen(true);
    }
    return _owner;
}

EntityBuilder& HudBuilder::anchorOffset(const graphic::Vector2f& offset)
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud)
        hud->setAnchorOffset(offset);
    return _owner;
}

EntityBuilder& HudBuilder::title(const std::string& text, float fontSize)
{
    auto hud = _entity->getBehavior<behavior::HudContainerBehavior>();
    if (hud)
        hud->setTitle(text, fontSize);
    return _owner;
}

EntityBuilder& HudBuilder::hidden()
{
    if (auto hud = _entity->getBehavior<behavior::HudContainerBehavior>())
        hud->setVisible(false, 0.f);
    return _owner;
}