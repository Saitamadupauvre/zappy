# Menu System

## Overview

`MenuScene` is the scene active during `AppState::Menu` and `AppState::Connecting`. It uses the same HUD entity system as `WorldScene` but has no 3D entities — it's a pure 2D panel stack.

```
MenuScene
  └── HudManager (_hud)
        ├── MainMenuPanel (ID 9880)  — main entry point
        ├── ConnectPanel  (ID 9881)  — host/port input
        └── LaunchPanel   (ID 9882)  — launch server config
```

## States

`MenuState` enum drives which panel is active:

| State | Visible panel | Description |
|---|---|---|
| `Main` | `MainMenuPanel` | Initial view — Connect, Launch, Quit buttons |
| `Connect` | `ConnectPanel` | Host/port input + connect button |
| `Launch` | `LaunchPanel` | Server executable config + launch button |

## Panels

### MainMenuPanel

Entry point. Three buttons:
- **Connect** → switches to `Connect` state
- **Launch server** → switches to `Launch` state
- **Quit** → exits application

### ConnectPanel

Inputs:
- Host field (`InputTextData`) — defaults to `localhost`
- Port field (`InputTextData`) — numeric only
- **Connect** button → `GameEngine::startConnecting(host, port)`
- **Back** button → returns to `Main`

Status line shows connection progress or error (`setConnectStatus(msg)`).

### LaunchPanel

Lets the user configure and spawn a `zappy_server` process locally:
- Port, map size (x/y), clients per team, team names, frequency
- **Launch** button → `ProcessSpawner::spawn(cmdLine)` then auto-connect

## Providers

Each panel owns a provider implementing `IHudProvider`:

| Provider | Panel |
|---|---|
| `MainMenuProvider` | `MainMenuPanel` |
| `ConnectProvider` | `ConnectPanel` |
| `LaunchProvider` | `LaunchPanel` |

## InputTextData in providers

Menu panels use `InputTextData` elements (see [HUD.md](HUD.md)) for text input. The `HudContainerBehavior` intercepts `KeyEvent` when an input field is focused and modifies the string in-place.

```cpp
InputTextData{
    .placeholder = "localhost",
    .value       = &_host,
    .onChange    = [this](const std::string& s){ _host = s; },
}
```

## Transition to game

`GameEngine` wires the connect callback:

```cpp
_menuScene->setOnConnect([this](std::string host, int port) {
    _state = AppState::Connecting;
    _network.connect(host, port);
});
```

On successful handshake, `GameEngine` calls `switchToGame()` which:
1. Sets `_state = AppState::InGame`
2. Replaces `_scene` with the fully initialized `WorldScene`
3. Calls `Locator::provide(_scene.get())`

## Adding a new menu screen

1. Write a provider in `src/scene/hud/menu/`.
2. Add a panel class with `setup(hud)` and a `show()` / `hide()` toggle.
3. Add a `MenuState` value.
4. Pick a free HUD entity ID (currently next free: 9883+).
5. Wire show/hide in `MenuScene`'s state transition logic.
