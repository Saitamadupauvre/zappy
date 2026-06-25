#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include <functional>
#include <cstdint>

namespace graphic { class Entity; }

class InteractionBuilder {
    public:
        using ClickCallback  = std::function<void(graphic::Entity&)>;
        using HoverCallback  = std::function<void(graphic::Entity&)>;
        using SelectCallback = std::function<void(graphic::Entity&, bool)>;

        InteractionBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& outline();
        EntityBuilder& onClick(ClickCallback cb);
        EntityBuilder& onHover(HoverCallback onEnter, HoverCallback onLeave);
        EntityBuilder& selectable(SelectCallback cb = [](graphic::Entity&, bool){});

        EntityBuilder& hoverScale(float baseScale = 1.0f, float factor = 1.2f);
        EntityBuilder& hoverScaleTag(uint32_t tagId, float baseScale = 1.0f, float factor = 1.2f);
        EntityBuilder& selectableOutline();

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
