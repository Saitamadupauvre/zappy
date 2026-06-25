#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "world/WorldTypes.hpp"
#include "graphic/Types.hpp"
#include <array>
#include <cstdint>
#include <string>

class InventoryProvider : public behavior::hud::IHudProvider {
public:
    static constexpr int RESOURCE_COUNT = 7;

    static constexpr const char* RESOURCE_NAMES[RESOURCE_COUNT] = {
        "Food", "Linemate", "Deraumere", "Sibur", "Mendiane", "Phiras", "Thystame"
    };

    void setPlayer(uint32_t id) {
        _playerId = id;
        _hasPlayer = true;
        for (auto& c : _counts) c = 0;
        _hasData = false;
        markDirty();
    }

    void setInventory(const zappy::Resources& inv) {
        _counts[0] = inv.food;
        _counts[1] = inv.linemate;
        _counts[2] = inv.deraumere;
        _counts[3] = inv.sibur;
        _counts[4] = inv.mendiane;
        _counts[5] = inv.phiras;
        _counts[6] = inv.thystame;
        _hasData = true;
        markDirty();
    }

    void setResourceTextures(std::array<graphic::TextureHandle, RESOURCE_COUNT> textures) {
        _textures = textures;
        markDirty();
    }

    void clear() { _hasPlayer = false; _hasData = false; markDirty(); }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        if (!_dirty) return _cache;
        _dirty = false;
        _cache.clear();
        if (!_hasPlayer) return _cache;

        _cache.reserve(9);
        for (int i = 0; i < 9; ++i) {
            behavior::hud::SlotData slot;
            slot.slotSize  = 52.f;
            slot.imageSize = 34.f;
            slot.fontSize  = 10.f;
            slot.bgColor     = {25, 25, 30, 230};
            slot.borderColor = {80, 80, 95, 255};
            slot.textColor   = {220, 220, 220, 255};
            if (i < RESOURCE_COUNT) {
                slot.texture = _textures[i];
                slot.label   = std::to_string(_counts[i]);
            }
            _cache.push_back({ slot });
        }
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    uint32_t _playerId = 0;
    int      _counts[RESOURCE_COUNT] = {};
    std::array<graphic::TextureHandle, RESOURCE_COUNT> _textures{};
    bool _hasPlayer = false;
    bool _hasData   = false;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable bool     _dirty   = true;
    mutable uint64_t _version = 1;

    void markDirty() { _dirty = true; ++_version; }
};
