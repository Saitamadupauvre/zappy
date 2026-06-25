#include "EntityManager.hpp"
#include <algorithm>
#include "behavior/selectable/SelectableBehavior.hpp"
#include "event/WorldEvent.hpp"

namespace zappy {

void EntityManager::addEntity(const std::shared_ptr<graphic::Entity>& entity)
{
    if (!entity) return;
    _entities.push_back(entity);
    _entityIndex[entity->getID()] = entity;
}

std::shared_ptr<graphic::Entity> EntityManager::createEntity(graphic::EntityID id, const std::string& type)
{
    auto entity = std::make_shared<graphic::Entity>(id, type);
    _entities.push_back(entity);
    _entityIndex[id] = entity;
    return entity;
}

void EntityManager::removeEntity(graphic::EntityID id)
{
    _entityIndex.erase(id);
    _entities.erase(
        std::remove_if(_entities.begin(), _entities.end(),
            [id](const std::shared_ptr<graphic::Entity>& entity) {
                return entity->getID() == id;
            }),
        _entities.end()
    );
}

std::shared_ptr<graphic::Entity> EntityManager::getEntity(graphic::EntityID id) const
{
    auto it = _entityIndex.find(id);
    return (it != _entityIndex.end()) ? it->second : nullptr;
}

const std::vector<std::shared_ptr<graphic::Entity>>& EntityManager::getEntities() const
{
    return _entities;
}

void EntityManager::update(float deltaTime)
{
    for (size_t i = 0; i < _entities.size(); ++i) {
        _entities[i]->update(deltaTime);
    }
}

void EntityManager::handleEvent(const event::Event& event)
{
    bool tileHandled = false;
    event::on(event,
        [&](const event::EntitySelectedEvent& e) {
            applySelection(e.entityId);
        },
        [&](const event::TileChangedEvent& tce) {
            uint64_t key = (static_cast<uint64_t>(tce.x) << 32) | static_cast<uint64_t>(tce.y);
            auto it = _tileListeners.find(key);
            if (it != _tileListeners.end()) {
                for (auto id : it->second) {
                    auto entity = getEntity(id);
                    if (entity) entity->handleEvent(event);
                }
            }
            tileHandled = true;
        },
        [&](const event::EntityMoveToEvent& e) {
            auto it = _entityIndex.find(e.entityId);
            if (it != _entityIndex.end()) it->second->handleEvent(event);
            tileHandled = true;
        },
        [&](const event::EntityRotateToEvent& e) {
            auto it = _entityIndex.find(e.entityId);
            if (it != _entityIndex.end()) it->second->handleEvent(event);
            tileHandled = true;
        },
        [&](const event::HoverEvent& e) {
            auto it = _entityIndex.find(e.entityId);
            if (it != _entityIndex.end()) it->second->handleEvent(event);
            tileHandled = true;
        }
    );

    if (tileHandled) return;

    const bool isRender = std::holds_alternative<event::RenderEvent>(event);
    for (size_t i = 0; i < _entities.size(); ++i) {
        if (isRender && _entities[i]->isDormant()) continue;
        _entities[i]->handleEvent(event);
    }
}

void EntityManager::applySelection(graphic::EntityID id)
{
    for (auto& entity : _entities) {
        const bool isTarget = entity->getID() == id;
        auto selectable = entity->getBehavior<behavior::SelectableBehavior>();

        if (selectable && selectable->isSelected() != isTarget)
            selectable->setSelected(*entity, isTarget);
        entity->handleEvent(event::SelectEvent{entity->getID(), isTarget});
    }
}

void EntityManager::clear()
{
    _entities.clear();
    _entityIndex.clear();
    _tileListeners.clear();
}

void EntityManager::registerTileListener(int x, int y, graphic::EntityID id)
{
    uint64_t key = (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
    _tileListeners[key].push_back(id);
}

void EntityManager::clearTileListeners()
{
    _tileListeners.clear();
}

} // namespace zappy
