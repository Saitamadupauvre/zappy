#pragma once
#include "behavior/ABehavior.hpp"
#include "behavior/hud/ResourceInfoProvider.hpp"
#include <memory>

namespace behavior {

class InfoHudBehavior : public ABehavior {
public:
    explicit InfoHudBehavior(std::shared_ptr<ResourceInfoProvider> provider);

    void onUpdate(graphic::Entity& owner, float deltaTime) override; // Obligatoire
    void onEvent(graphic::Entity& owner, const event::Event& ev) override; // Override utile

private:
    std::shared_ptr<ResourceInfoProvider> _provider;
};

} // namespace behavior