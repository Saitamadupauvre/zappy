#pragma once

#include "ISettingsSection.hpp"
#include <functional>

namespace zappy {

class GameDisplaySection : public ISettingsSection {
public:
    void setOnIncantationEffect(std::function<void(bool)> fn)   { _onIncantation   = std::move(fn); }
    void setOnBroadcastCircle(std::function<void(bool)> fn)     { _onBroadcast     = std::move(fn); }
    void setOnEggHatchAnim(std::function<void(bool)> fn)        { _onEggHatch      = std::move(fn); }
    void setOnTeamColorTags(std::function<void(bool)> fn)       { _onTeamColor     = std::move(fn); }
    void setOnSkyMode(std::function<void(int)> fn)              { _onSkyMode       = std::move(fn); }
    void setOnGrass(std::function<void(bool)> fn)               { _onGrass         = std::move(fn); }

    std::string sectionTitle() const override { return "Game Display"; }
    std::vector<behavior::hud::HudElement> getHudElements() const override;

private:
    static constexpr const char* SKY_OPTIONS[] = { "Shader", "Empty" };

    mutable bool _incantation    = true;
    mutable bool _broadcastCircle = true;
    mutable bool _eggHatchAnim   = true;
    mutable bool _teamColorTags  = true;
    mutable bool _grass          = true;
    mutable int  _skyIdx         = 0;
    mutable bool _skyOpen        = false;

    std::function<void(bool)> _onIncantation;
    std::function<void(bool)> _onBroadcast;
    std::function<void(bool)> _onEggHatch;
    std::function<void(bool)> _onTeamColor;
    std::function<void(int)>  _onSkyMode;
    std::function<void(bool)> _onGrass;
};

} // namespace zappy
