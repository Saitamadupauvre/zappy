#pragma once

#include "entity/Entity.hpp"
#include "event/Event.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace zappy {

class EntityManager
{
    public:
        EntityManager() = default;
        ~EntityManager() = default;

        /**
         * @brief Adds an existing entity to the manager.
         * @param entity The entity to add.
         */
        void addEntity(const std::shared_ptr<graphic::Entity>& entity);

        /**
         * @brief Creates and adds a new entity.
         * @param id The unique ID of the entity.
         * @param type The type of the entity.
         * @return The created entity.
         */
        std::shared_ptr<graphic::Entity> createEntity(graphic::EntityID id, const std::string& type);

        /**
         * @brief Removes an entity by its ID.
         * @param id The unique ID of the entity to remove.
         */
        void removeEntity(graphic::EntityID id);

        /**
         * @brief Retrieves an entity by its ID.
         * @param id The unique ID of the entity.
         * @return The entity if found, nullptr otherwise.
         */
        std::shared_ptr<graphic::Entity> getEntity(graphic::EntityID id) const;

        /**
         * @brief Gets all entities.
         * @return A list of all entities.
         */
        const std::vector<std::shared_ptr<graphic::Entity>>& getEntities() const;

        /**
         * @brief Updates all managed entities.
         * @param deltaTime The elapsed time since the last frame.
         */
        void update(float deltaTime);

        /**
         * @brief Dispatches events to all managed entities.
         * @param event The window event to handle.
         */
        void handleEvent(const event::Event& event);

        /**
         * @brief Clears all managed entities.
         */
        void clear();

        void registerTileListener(int x, int y, graphic::EntityID id);
        void clearTileListeners();

    private:
        /**
         * @brief Marks the entity matching @p id as selected and clears the
         *        selection on every other managed entity.
         */
        void applySelection(graphic::EntityID id);

        std::vector<std::shared_ptr<graphic::Entity>>                            _entities;
        std::unordered_map<graphic::EntityID, std::shared_ptr<graphic::Entity>> _entityIndex;
        std::unordered_map<uint64_t, std::vector<graphic::EntityID>>             _tileListeners;
};

} // namespace zappy
