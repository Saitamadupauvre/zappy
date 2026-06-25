#include "DrawableBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/drawable/mesh/MeshDrawableBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "behavior/drawable/tag/TextDrawableBehavior.hpp"

DrawableBuilder::DrawableBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& DrawableBuilder::mesh(graphic::MeshHandle mesh,
                                     graphic::TextureHandle texture,
                                     graphic::Color4b tint)
{
    _entity->addBehavior<behavior::MeshDrawableBehavior>(mesh, texture, tint);
    return _owner;
}

EntityBuilder& DrawableBuilder::model(graphic::IRenderer& renderer,
                                      graphic::ModelHandle model,
                                      graphic::Color4b tint,
                                      const graphic::Vector3f& rotationOffset,
                                      bool ownsModel)
{
    auto drawable = _entity->addBehavior<behavior::ModelDrawableBehavior>(renderer, model, tint, ownsModel);
    drawable->setRotationOffset(rotationOffset);
    return _owner;
}

EntityBuilder& DrawableBuilder::text(const std::string& text, float size,
                                     graphic::Color4b color, graphic::Anchor anchor)
{
    _entity->addBehavior<behavior::TextDrawableBehavior>(text, size, color, anchor);
    return _owner;
}

EntityBuilder& DrawableBuilder::meshVisible(bool visible)
{
    if (auto d = _entity->getBehavior<behavior::MeshDrawableBehavior>())
        d->setVisible(visible);
    if (auto d = _entity->getBehavior<behavior::ModelDrawableBehavior>())
        d->setVisible(visible);
    return _owner;
}

EntityBuilder& DrawableBuilder::textVisible(bool visible)
{
    if (auto d = _entity->getBehavior<behavior::TextDrawableBehavior>())
        d->setVisible(visible);
    return _owner;
}
