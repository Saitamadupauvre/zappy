#include "EntityBuilder.hpp"
#include "BuildersIncludes.hpp"
 
EntityBuilder::EntityBuilder(EntityManager& em, uint32_t id, const std::string& type)
    : _em(em) 
{
    _entity = _em.createEntity(id, type);
}

EntityBuilder::~EntityBuilder() = default;

TransformBuilder& EntityBuilder::transform() {
    if (!_transform) _transform = std::make_unique<TransformBuilder>(*this, _entity);
    return *_transform;
}

MovementBuilder& EntityBuilder::movement() {
    if (!_movement) _movement = std::make_unique<MovementBuilder>(*this, _entity);
    return *_movement;
}

InteractionBuilder& EntityBuilder::interaction() {
    if (!_interaction) _interaction = std::make_unique<InteractionBuilder>(*this, _entity);
    return *_interaction;
}

DrawableBuilder& EntityBuilder::drawable() {
    if (!_drawable) _drawable = std::make_unique<DrawableBuilder>(*this, _entity);
    return *_drawable;
}

PlayerBuilder& EntityBuilder::player() {
    if (!_player) _player = std::make_unique<PlayerBuilder>(*this, _entity);
    return *_player;
}

ResourceBuilder& EntityBuilder::resource() {
    if (!_resource) _resource = std::make_unique<ResourceBuilder>(*this, _entity);
    return *_resource;
}

EggBuilder& EntityBuilder::egg() {
    if (!_egg) _egg = std::make_unique<EggBuilder>(*this, _entity);
    return *_egg;
}

TagBuilder& EntityBuilder::tag() {
    if (!_tag) _tag = std::make_unique<TagBuilder>(*this, _entity);
    return *_tag;
}

HudBuilder& EntityBuilder::hud() {
    if (!_hud) _hud = std::make_unique<HudBuilder>(*this, _entity);
    return *_hud;
}

EntityPtr EntityBuilder::build() {
    return _entity;
}