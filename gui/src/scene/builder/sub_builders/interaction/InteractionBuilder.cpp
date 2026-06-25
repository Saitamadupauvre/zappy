#include "InteractionBuilder.hpp"
#include "entity/Entity.hpp"
#include "behavior/clickable/ClickableBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "behavior/selectable/SelectableBehavior.hpp"
#include "behavior/outline/OutlineBehavior.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/drawable/tag/TextDrawableBehavior.hpp"
#include "scene/IScene.hpp"
#include "locator/Locator.hpp"

InteractionBuilder::InteractionBuilder(EntityBuilder& owner, EntityPtr entity)
    : _owner(owner), _entity(entity)
{
}

EntityBuilder& InteractionBuilder::outline()
{
    _entity->addBehavior<behavior::OutlineBehavior>();
    return _owner;
}

EntityBuilder& InteractionBuilder::onClick(ClickCallback cb)
{
    _entity->addBehavior<behavior::ClickableBehavior>(std::move(cb));
    return _owner;
}

EntityBuilder& InteractionBuilder::onHover(HoverCallback onEnter, HoverCallback onLeave)
{
    _entity->addBehavior<behavior::HoverableBehavior>(std::move(onEnter), std::move(onLeave));
    return _owner;
}

EntityBuilder& InteractionBuilder::selectable(SelectCallback cb)
{
    _entity->addBehavior<behavior::SelectableBehavior>(std::move(cb));
    return _owner;
}

EntityBuilder& InteractionBuilder::hoverScale(float baseScale, float factor)
{
    _entity->addBehavior<behavior::HoverableBehavior>(
        [baseScale, factor](graphic::Entity& e) {
            if (auto outline = e.getBehavior<behavior::OutlineBehavior>())
                outline->setHovered(true);
            if (auto t = e.getBehavior<behavior::TransformBehavior>())
                t->setScale({baseScale * factor, baseScale * factor, baseScale * factor});
        },
        [baseScale](graphic::Entity& e) {
            if (auto outline = e.getBehavior<behavior::OutlineBehavior>())
                outline->setHovered(false);
            if (auto t = e.getBehavior<behavior::TransformBehavior>())
                t->setScale({baseScale, baseScale, baseScale});
        });
    return _owner;
}

EntityBuilder& InteractionBuilder::hoverScaleTag(uint32_t tagId, float baseScale, float factor)
{
    auto setTagVisible = [tagId](bool visible) {
        auto tag = zappy::Locator::getScene()->getHud().getEntity(tagId);
        if (tag)
            if (auto text = tag->getBehavior<behavior::TextDrawableBehavior>())
                text->setVisible(visible);
    };
    _entity->addBehavior<behavior::HoverableBehavior>(
        [baseScale, factor, setTagVisible](graphic::Entity& e) {
            if (auto outline = e.getBehavior<behavior::OutlineBehavior>())
                outline->setHovered(true);
            if (auto t = e.getBehavior<behavior::TransformBehavior>())
                t->setScale({baseScale * factor, baseScale * factor, baseScale * factor});
            setTagVisible(true);
        },
        [baseScale, setTagVisible](graphic::Entity& e) {
            if (auto outline = e.getBehavior<behavior::OutlineBehavior>())
                outline->setHovered(false);
            if (auto t = e.getBehavior<behavior::TransformBehavior>())
                t->setScale({baseScale, baseScale, baseScale});
            setTagVisible(false);
        });
    return _owner;
}

EntityBuilder& InteractionBuilder::selectableOutline()
{
    _entity->addBehavior<behavior::SelectableBehavior>(
        [](graphic::Entity& e, bool isSelected) {
            if (auto outline = e.getBehavior<behavior::OutlineBehavior>())
                outline->setSelected(isSelected);
        });
    return _owner;
}
