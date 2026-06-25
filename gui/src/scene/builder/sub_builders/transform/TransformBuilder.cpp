#include "TransformBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/transform/TransformBehavior.hpp"

TransformBuilder::TransformBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

std::shared_ptr<behavior::TransformBehavior> TransformBuilder::ensure()
{
    auto t = _entity->getBehavior<behavior::TransformBehavior>();
    if (!t)
        t = _entity->addBehavior<behavior::TransformBehavior>();
    return t;
}

EntityBuilder& TransformBuilder::position(const graphic::Vector3f& pos)
{
    ensure()->setPosition(pos);
    return _owner;
}

EntityBuilder& TransformBuilder::rotation(const graphic::Vector3f& rot)
{
    ensure()->setRotation(rot);
    return _owner;
}

EntityBuilder& TransformBuilder::scale(const graphic::Vector3f& scale)
{
    ensure()->setScale(scale);
    return _owner;
}

EntityBuilder& TransformBuilder::scale(float uniform)
{
    ensure()->setScale({uniform, uniform, uniform});
    return _owner;
}

EntityBuilder& TransformBuilder::orientation(const graphic::Vector3f& up, float yawDeg)
{
    ensure()->setOrientation(up, yawDeg);
    return _owner;
}
