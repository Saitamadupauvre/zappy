#include "EntityManager.hpp"
#include <algorithm>

namespace zappy {

void EntityManager::addEntity(const std::shared_ptr<graphic::Entity>& entity)
{
    if (!entity) return;
    _entities.push_back(entity);
}

std::shared_ptr<graphic::Entity> EntityManager::createEntity(graphic::EntityID id, const std::string& type)
{
    auto entity = std::make_shared<graphic::Entity>(id, type);
    _entities.push_back(entity);
    return entity;
}

void EntityManager::removeEntity(graphic::EntityID id)
{
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
    for (const auto& entity : _entities) {
        if (entity->getID() == id) {
            return entity;
        }
    }
    return nullptr;
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
    for (size_t i = 0; i < _entities.size(); ++i) {
        _entities[i]->handleEvent(event);
    }
}

void EntityManager::clear()
{
    _entities.clear();
}

} // namespace zappy
