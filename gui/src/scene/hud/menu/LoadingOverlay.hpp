#pragma once

#include "core/manager/hud/HudManager.hpp"
#include "behavior/hud/HudContainerBehavior.hpp"
#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "graphic/Types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace zappy {

class LoadingProvider : public behavior::hud::IHudProvider {
public:
    void setProgress(float ratio) { _ratio = ratio; markDirty(); }
    void setText(const std::string& t) { _text = t; markDirty(); }

    void markDirty() { ++_version; _dirty = true; }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override;
    uint64_t getVersion() const override { return _version; }

private:
    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    std::string _text  = "Loading world…";
    float       _ratio = 0.f;
};

class LoadingOverlay {
public:
    static constexpr graphic::EntityID ID = 9879;

    void setup(HudManager& hud);

    void setProgress(float ratio);
    void setText(const std::string& text);
    void show();
    void hide();
    bool isVisible() const;

private:
    std::shared_ptr<LoadingProvider>              _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;
};

} // namespace zappy
