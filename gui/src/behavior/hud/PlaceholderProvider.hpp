#include "hud/IHudProvider.hpp"

using namespace behavior::hud;

class PlaceholderProvider : public IHudProvider {
public:
    const std::vector<HudElement>& getHudElements() const override;

private:
    mutable std::vector<HudElement> _cache;
};
