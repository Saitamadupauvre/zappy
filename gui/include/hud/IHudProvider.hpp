#pragma once

#include "HudElements.hpp"
#include <cstdint>
#include <vector>

namespace behavior::hud {

class IHudProvider {
public:
    virtual ~IHudProvider() = default;

    virtual const std::vector<HudElement>& getHudElements() const = 0;
    virtual uint64_t getVersion() const { return 0; }
};

} // namespace behavior::hud