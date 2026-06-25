#pragma once

#include "BehaviorBuilders.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include <string>
#include <memory>
#include <cstdint>

using EntityManager = zappy::EntityManager;

class EntityBuilder {
    public:
        EntityBuilder(EntityManager& em, uint32_t id, const std::string& type);
        ~EntityBuilder();

        TransformBuilder&   transform();
        MovementBuilder&    movement();
        InteractionBuilder& interaction();
        DrawableBuilder&    drawable();
        PlayerBuilder&      player();
        ResourceBuilder&    resource();
        EggBuilder&         egg();
        TagBuilder&         tag();
        HudBuilder&         hud();

        EntityPtr build();
    
    private:
        EntityPtr _entity;
        EntityManager& _em;
        
        std::unique_ptr<TransformBuilder>   _transform;
        std::unique_ptr<MovementBuilder>    _movement;
        std::unique_ptr<InteractionBuilder> _interaction;
        std::unique_ptr<DrawableBuilder>    _drawable;
        std::unique_ptr<PlayerBuilder>      _player;
        std::unique_ptr<ResourceBuilder>    _resource;
        std::unique_ptr<EggBuilder>         _egg;
        std::unique_ptr<TagBuilder>         _tag;
        std::unique_ptr<HudBuilder>         _hud;
};