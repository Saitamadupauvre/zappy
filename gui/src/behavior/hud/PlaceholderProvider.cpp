#include "PlaceholderProvider.hpp"

const std::vector<HudElement>& PlaceholderProvider::getHudElements() const {
    _cache.clear();

    _cache.push_back({
        TextData{ "HUD DE TEST", 30.0f, graphic::Color4b::yellow() }
    });
    _cache.push_back({
        TextData{ "FPS: 60", 20.0f, graphic::Color4b::gray() }
    });
    _cache.push_back({
        BarData{ 0.75f, graphic::Color4b::green(), 200.0f, 15.0f }
    });

    return _cache;
}
