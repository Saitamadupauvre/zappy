#pragma once

#include "ISettingsSection.hpp"
#include <functional>
#include <tuple>

namespace zappy {

class VideoSection : public ISettingsSection {
public:
    void setOnFpsChange(std::function<void(int)> fn)       { _onFpsChange  = std::move(fn); }
    void setOnFovChange(std::function<void(float)> fn)     { _onFov        = std::move(fn); }
    void setOnFpsOverlay(std::function<void(bool)> fn)     { _onFpsOverlay = std::move(fn); }
    void setOnFullscreen(std::function<void(bool)> fn)     { _onFullscreen = std::move(fn); }
    void setOnResolution(std::function<void(int,int)> fn)  { _onResolution = std::move(fn); }

    std::string sectionTitle() const override { return "Video"; }
    std::vector<behavior::hud::HudElement> getHudElements() const override;

private:
    static constexpr std::pair<const char*, int> FPS_OPTIONS[] = {
        {"30",        30},
        {"60",        60},
        {"120",      120},
        {"144",      144},
        {"240",      240},
        {"Unlimited",  0},
    };

    static constexpr std::tuple<const char*, int, int> RES_OPTIONS[] = {
        {"800 x 600",   800,  600},
        {"1024 x 768", 1024,  768},
        {"1280 x 720", 1280,  720},
        {"1280 x 800", 1280,  800},
        {"1366 x 768", 1366,  768},
        {"1440 x 900", 1440,  900},
        {"1600 x 900", 1600,  900},
        {"1680 x 1050",1680, 1050},
        {"1920 x 1080",1920, 1080},
        {"2560 x 1440",2560, 1440},
        {"3840 x 2160",3840, 2160},
    };

    mutable int   _fpsIdx      = 1;
    mutable bool  _fpsOpen     = false;
    mutable int   _resIdx      = 1;
    mutable bool  _resOpen     = false;
    mutable bool  _fullscreen  = false;
    mutable float _fov         = 60.f;
    mutable bool  _fpsOverlay  = false;

    std::function<void(int)>      _onFpsChange;
    std::function<void(float)>    _onFov;
    std::function<void(bool)>     _onFpsOverlay;
    std::function<void(bool)>     _onFullscreen;
    std::function<void(int,int)>  _onResolution;
};

} // namespace zappy
