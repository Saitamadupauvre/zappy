# HUD System

## Overview

HUD entities live in `_hud` (a `HudManager`, typedef of `EntityManager`). They are rendered in a separate 2D pass after the 3D world. The system uses the **Provider pattern**: a panel entity holds a `HudContainerBehavior` that calls `IHudProvider::getHudElements()` every frame to get the list of elements to draw.

```
HUD entity
  └── HudContainerBehavior
        └── IHudProvider::getHudElements() → [TextData, ButtonData, ...]
```

`HudPicker` intercepts mouse events before the 3D `PickSystem` so clicking a HUD panel doesn't also click a world entity behind it.

## Coordinate system

All pixel values are authored at the **1280 × 720** reference resolution. `HudContainerBehavior` scales everything by `_uiScale = viewport.x / 1280.f` (clamped to ≥ 0.5) at runtime. Do **not** multiply by screen size manually — just use design-pixel values.

## Creating a HUD panel

### 1. Write a provider

```cpp
#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"

class MyProvider : public behavior::hud::IHudProvider {
public:
    void setData(int value) { _value = value; }

    std::vector<behavior::hud::HudElement> getHudElements() const override {
        using namespace behavior::hud;
        return {
            HudElement{TextData{"Score: " + std::to_string(_value), 16.f}},
            HudElement{ButtonData{
                .label   = "Reset",
                .onClick = [this]{ /* callback */ },
            }},
        };
    }

private:
    int _value = 0;
};
```

`getHudElements()` is called every frame — keep it cheap. Use cached state; don't query heavy data structures here.

### 2. Create the HUD entity

```cpp
auto provider = std::make_shared<MyProvider>();

EntityBuilder(_hud, MY_PANEL_ID, "my_panel")
    .hud().container(provider)
    .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 8.f)
    .hud().anchor(graphic::Anchor::TopRight)
    .hud().anchorOffset({10.f, 50.f})
    .hud().background(true,
        graphic::Color4b{20, 20, 20, 210},   // fill
        graphic::Color4b{80, 80, 80, 255})   // border
    .hud().boxSize({280.f, 200.f})
    .hud().title("My Panel", 13.f)
    .build();
```

### 3. Show / hide at runtime

```cpp
auto* hud = _hud.getEntity(MY_PANEL_ID);
if (hud) {
    auto b = hud->getBehavior<behavior::HudContainerBehavior>();
    if (b) b->setVisible(!b->isFullyVisible());
}
```

Use `isFullyVisible()` (animState == Visible) for toggle logic — **not** `isVisible()` (which returns true even while fading in).

## HUD element types

All types live in `include/hud/HudElements.hpp`.

### TextData
```cpp
TextData{
    .content  = "Hello",
    .fontSize = 16.f,
    .color    = graphic::Color4b::white(),
}
```

### BarData
```cpp
BarData{
    .ratio     = 0.75f,         // 0.0–1.0
    .fillColor = {0, 200, 80, 255},
    .width     = 150.f,
    .height    = 12.f,
}
```

### RectData
```cpp
RectData{
    .width       = 100.f,
    .height      = 40.f,
    .borderColor = {100, 100, 100, 255},
    .fillColor   = {30, 30, 30, 200},
}
```

### ButtonData
```cpp
ButtonData{
    .label        = "Click me",
    .fontSize     = 14.f,
    .width        = 130.f,
    .height       = 28.f,
    .textColor    = {255, 255, 255, 255},
    .bgColor      = {40,  110, 210, 220},
    .hoverBgColor = {70,  145, 255, 230},
    .onClick      = []{ /* handle click */ },
}
```

No separate entity needed per button. `HudContainerBehavior` handles hit-testing and calls `onClick` internally.

### ChatBubbleData
```cpp
ChatBubbleData{
    .text        = "message",
    .sender      = "PlayerName",
    .isLeft      = true,           // left bubble = other player
    .bubbleColor = {40, 80, 180, 220},
    .maxWidth    = 240.f,
    .fontSize    = 13.f,
}
```

### ImageData
```cpp
ImageData{
    .texture = textureHandle,
    .width   = 64.f,
    .height  = 64.f,
    .opacity = 1.f,
    .tint    = graphic::Color4b::white(),
}
```

### SliderData
```cpp
SliderData{
    .min       = 1.f,
    .max       = 100.f,
    .value     = 42.f,
    .width     = 200.f,
    .height    = 14.f,
    .onChange  = [](float v){ /* visual update during drag */ },
    .onRelease = [](float v){ /* commit on mouse-up */ },
}
```

`onChange` fires every frame during drag. `onRelease` fires once on mouse button release. Use `onRelease` to send server commands (e.g. `sst`).

### SlotData
```cpp
SlotData{
    .texture     = iconHandle,
    .label       = "5",           // shown below the icon
    .slotSize    = 52.f,
    .imageSize   = 36.f,
    .fontSize    = 10.f,
    .bgColor     = {25, 25, 25, 230},
    .borderColor = {90, 90, 90, 255},
    .textColor   = {255, 255, 255, 255},
}
```

Used for inventory-style grids.

### InputTextData
```cpp
InputTextData{
    .placeholder = "localhost",
    .value       = &_hostString,   // pointer to string modified in place
    .fontSize    = 14.f,
    .width       = 200.f,
    .height      = 28.f,
    .onChange    = [this](const std::string& s){ _host = s; },
}
```

Used in `ConnectPanel` for host/port fields. The behavior handles keyboard capture internally.

## Layout types

Set with `.hud().layout(type, padding)`:

| Type | Behavior |
|---|---|
| `Vertical` (default) | Elements stacked top-to-bottom, center-aligned |
| `Horizontal` | Elements placed left-to-right |
| `MediaObject` | Element[0] in left column; elements[1..N] stacked in right column (leaderboard rows) |
| `VerticalMedia` | Vertical with an image/media pinned to top |
| `Grid` | Elements arranged in a configurable-column grid |
| `MediaObjectHButtons` | Like MediaObject but right column uses horizontal button layout |

## Anchor positions

```cpp
graphic::Anchor::TopLeft
graphic::Anchor::TopCenter
graphic::Anchor::TopRight
graphic::Anchor::MiddleLeft
graphic::Anchor::MiddleCenter
graphic::Anchor::MiddleRight
graphic::Anchor::BottomLeft
graphic::Anchor::BottomCenter
graphic::Anchor::BottomRight
```

`.hud().anchorOffset({dx, dy})` shifts from the anchor in design-pixels (auto-scaled).

## Scrollable panels

```cpp
.hud().container(provider)
// then after build:
auto b = entity->getBehavior<behavior::HudContainerBehavior>();
b->setScrollable(true);
b->scrollToBottom();  // after adding new messages
```

## World-space tags

Tags that float above 3D entities use `TagBehavior` to track world position and `.hud().isWorldSpaceTag(true)` so the container doesn't use anchor-based positioning:

```cpp
EntityBuilder(_hud, playerId + 10000, "player_tag")
    .hud().container(provider)
    .hud().isWorldSpaceTag(true)
    .build();

// TagBehavior on the 3D entity writes position each frame:
entity->addBehavior<TagBehavior>(tagEntityId, _hud, offsetY);
```

## Visibility animation

`HudContainerBehavior` has slide+fade animation with states: `Hidden → FadingIn → Visible → FadingOut → Hidden`.

```cpp
b->setVisible(true);          // fade in (default duration)
b->setVisible(false);         // fade out
b->setVisible(true, 0.0f);    // instant show
b->isFullyVisible()           // true only in Visible state
b->isVisible()                // true during FadingIn and Visible
```

Start hidden in the builder with `.hud().hidden()`.

## Existing panels

### WorldScene panels

| Panel | ID | File | Purpose |
|---|---|---|---|
| `PlayerInfoPanel` | 9998 | `hud/player/` | Selected player name, level, team, position |
| `InventoryPanel` | 9890 | `hud/inventory/` | Selected player resources as icon slots |
| `ChatPanel` | 9997 | `hud/chat/` | Team broadcast chat bubbles |
| `LeaderboardPanel` | 9900–9949 | `hud/leaderboard/` | Team ranking (one entity per team) |
| `TeamDetailPanel` | 9896 | `hud/leaderboard/` | Per-team player list with Follow buttons |
| `PopupPanel` | 9893–9895 | `hud/popup/` | Toast notifications (3 rotating slots) |
| `SpeedPanel` | 9892 | `hud/speed/` | Server time unit slider |
| `SettingsPanel` | 9891 | `hud/settings/` | Keybindings remapper + video/audio options |
| `ResourceInfoPanel` | 9999 | `hud/resource/` | Tile resource counts on click |
| `ClockPanel` | 9889 | `hud/clock/` | Server uptime display (MM:SS:CS) |
| `WorldInfoPanel` | 9888 | `hud/world/` | World stats (map size, player count, teams) |
| `TeamStatsPanel` | — | `hud/leaderboard/` | Per-team aggregate stats overlay |

### MenuScene panels

| Panel | ID | File | Purpose |
|---|---|---|---|
| `MainMenuPanel` | 9880 | `hud/menu/` | Main menu buttons (Connect, Launch, Quit) |
| `ConnectPanel` | 9881 | `hud/menu/` | Host/port input + connect button |
| `LaunchPanel` | 9882 | `hud/menu/` | Server launch configuration |

## SceneHudManager

`SceneHudManager` owns and coordinates all WorldScene panels. It exposes a narrow interface:

```cpp
_hudMgr.setup(hud, entities, renderer, inputMgr, sendLine);
_hudMgr.onTeamAdded(team);
_hudMgr.onPlayerAdded(player);
_hudMgr.onPlayerLevelChanged(id, level);
_hudMgr.onBroadcast(id, msg);
_hudMgr.onEntitySelected(id);
_hudMgr.onInventoryChanged(id, inv);
_hudMgr.onTileClicked(x, y, tile);
_hudMgr.onTimeUnitChanged(tu);
_hudMgr.tick(dt);

// Panel accessors for callback wiring:
_hudMgr.leaderboard()
_hudMgr.teamDetail()
_hudMgr.chat()
_hudMgr.playerInfo()
_hudMgr.speedPanel()
_hudMgr.settingsPanel()
```

Wire panel callbacks in `WorldScene`'s constructor after `_hudMgr.setup(...)`:

```cpp
_hudMgr.teamDetail().setOnFollowClick([this](uint32_t id) {
    handleEvent(event::Event{event::LogicEvent{event::EntitySelectedEvent{id}}});
    if (!_camera.isFollowing())
        _camCtrl.onFollowToggle(_selectedPlayerId, _entities, _hud);
});
```

## Adding a new panel

1. Pick a free entity ID (see [ARCHITECTURE.md](ARCHITECTURE.md#entity-id-allocation)).
2. Write a provider in `src/scene/hud/<feature>/`.
3. Write a panel class that owns the provider and exposes `setup()` + event methods.
4. Add it to `SceneHudManager` (or directly in `WorldScene` for standalone panels).
5. Wire callbacks in `WorldScene`'s constructor after `_hudMgr.setup(...)`.
6. Hook into `SceneHudManager::onEvent` if it reacts to `WorldEvent`s.
