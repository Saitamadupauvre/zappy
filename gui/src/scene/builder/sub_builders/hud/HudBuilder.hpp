#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include "graphic/Vectors.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "hud/IHudProvider.hpp"
#include "behavior/hud/LayoutEngine.hpp"
#include "behavior/hud/ResourceInfoProvider.hpp"
#include <memory>

class HudBuilder {
    public:
        HudBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& rect(const graphic::Vector2f& pos, const graphic::Vector2f& size);

        EntityBuilder& container(std::shared_ptr<behavior::hud::IHudProvider> provider);

        
        EntityBuilder& layout(behavior::hud::LayoutEngine::Type type, float padding = 8.0f, int groupStride = 0);

        EntityBuilder& infoHud(std::shared_ptr<ResourceInfoProvider> provider);
    
        EntityBuilder& anchor(graphic::Anchor anchor);

        EntityBuilder& background(bool enabled, graphic::Color4b fill, graphic::Color4b border);

        EntityBuilder& isWorldSpaceTag(bool isWorldSpace);
        EntityBuilder& boxSize(const graphic::Vector2f& size);
        EntityBuilder& autoSize();
        EntityBuilder& fullscreen();
        EntityBuilder& anchorOffset(const graphic::Vector2f& offset);
        EntityBuilder& title(const std::string& text, float fontSize = 14.0f);
        EntityBuilder& hidden();

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};