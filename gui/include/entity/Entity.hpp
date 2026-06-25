#pragma once
#include "behavior/IBehavior.hpp"
#include "event/Event.hpp"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace graphic {

using EntityID = unsigned int;

class Entity
{
    public:
        Entity(EntityID id, const std::string& type) : _id(id), _type(type) {}
        ~Entity() = default;

        EntityID getID() const { return _id; }
        const std::string& getType() const { return _type; }

        template<typename T, typename... Args>
        std::shared_ptr<T> addBehavior(Args&&... args) {
            auto behavior = std::make_shared<T>(std::forward<Args>(args)...);
            _behaviors.push_back(behavior);
            behavior->onAttach(*this);
            return behavior;
        }

        template<typename T>
        std::shared_ptr<T> getBehavior() {
            for (auto& behavior : _behaviors) {
                auto casted = std::dynamic_pointer_cast<T>(behavior);
                if (casted) {
                    return casted;
                }
            }
            return nullptr;
        }

        template<typename T>
        bool hasBehavior() const {
            for (const auto& behavior : _behaviors) {
                if (std::dynamic_pointer_cast<T>(behavior)) {
                    return true;
                }
            }
            return false;
        }
        
        template<typename T>
        void removeBehavior() {
            _behaviors.erase(
                std::remove_if(_behaviors.begin(), _behaviors.end(),
                    [](const std::shared_ptr<behavior::IBehavior>& behavior) {
                        return std::dynamic_pointer_cast<T>(behavior) != nullptr;
                    }),
                _behaviors.end()
            );
        }

        void update(float deltaTime) {
            for (size_t i = 0; i < _behaviors.size(); ++i) {
                _behaviors[i]->onUpdate(*this, deltaTime);
            }
        }

        void handleEvent(const event::Event& event) {
            for (size_t i = 0; i < _behaviors.size(); ++i) {
                _behaviors[i]->onEvent(*this, event);
            }
        }

        void setDormant(bool d) { _dormant = d; }
        bool isDormant() const { return _dormant; }

        bool operator==(const Entity& other) const { return _id == other._id; }
        bool operator!=(const Entity& other) const { return !(*this == other); }
        bool operator==(const EntityID& id) const { return _id == id; }
        bool operator!=(const EntityID& id) const { return !(*this == id); }

    private:
        EntityID _id;
        std::string _type;
        bool _dormant = false;
        std::vector<std::shared_ptr<behavior::IBehavior>> _behaviors;
};

} // namespace graphic
