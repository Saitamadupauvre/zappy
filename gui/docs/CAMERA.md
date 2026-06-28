# Camera & Map Layout System

## OrbitCamera

`OrbitCamera` (`src/scene/camera/`) is a state machine with four modes:

| Mode | Description |
|---|---|
| `Center` | Orbits around the map center. Default mode. |
| `Follow` | Orbits around a moving target (a player entity). |
| `Free` | FPS-style free camera, not locked to any target. |
| `FirstPerson` | Locked to a player's eye position and forward vector. |

Key parameters (all public):

```
yaw      — horizontal angle (degrees)
pitch    — vertical angle (degrees, clamped)
distance — distance from target (orbit modes)
fov      — vertical field of view (degrees)
```

### Mode transitions

```cpp
camera.enterCenter();                       // orbit map center
camera.enterFollow(targetPos);              // orbit a player
camera.enterFirstPerson(pos, fwd, up);      // attach to player eye
camera.updateFollowTarget(newPos);          // called each frame in Follow/FirstPerson
```

`toCameraState()` converts internal state to a `CameraState` struct passed to the renderer.

## CameraController

`CameraController` wraps `OrbitCamera` and implements input logic:

```cpp
// Toggle follow / first-person (called on F key)
_camCtrl.onFollowToggle(playerId, _entities, _hud);
// Double-tap F → first-person; single-tap → follow/unfollow

// Switch follow target when entity is selected
_camCtrl.onEntitySelected(id, _entities);
```

**Double-tap threshold:** `DOUBLE_TAP_MS = 400` ms. First tap enters Follow, second tap within threshold enters FirstPerson.

**FPV eye height:** `FPV_EYE_HEIGHT = 0.55f` world units above the entity's transform position.

`applyLayoutFraming()` — called after a layout switch (Grid ↔ Torus) to reset the camera to the new map framing. Uses `IMapLayout::cameraFraming(w, h)`.

## Map layouts

Three pluggable geometries implement `IMapLayout`:

### GridLayout

Flat 2D grid. Default layout.

- `tilePos(x, y)` → `{x * spacing, 0, y * spacing}`
- `upAt()` → always `{0, 1, 0}`
- Players stand at `y = 0`, stacked vertically when multiple on same tile.
- Wrapping animations: disabled (`animatesWrap() = false`)

### TorusLayout

Toroidal surface (wrapping on both axes). Toggled with **G** key.

- Major radius `_R`, minor radius `_r` — computed from map size to fill the torus surface.
- `tilePos(x, y)` → point on torus surface using angle parameterization.
- `upAt(x, y)` → outward surface normal at that angle.
- `forwardAt(x, y)` → tangent along the longitude direction.
- Wrapping animations: enabled (`animatesWrap() = true`) — movement behavior interpolates on the torus surface.

### SphereLayout

Spherical surface mapping.

- Players stand on the outside of a sphere.
- `upAt(x, y)` → outward radial normal.

### Using layouts

```cpp
// WorldScene switches layout on MapLayoutCycleEvent:
_currentLayout = (isGrid) ? std::make_unique<TorusLayout>() : std::make_unique<GridLayout>();
_currentLayout->updateSizing(_worldW, _worldH);

// Pass to factories and systems:
_worldBuilder.build(_world, _entities, *_currentLayout);
_tileSystem.setActiveLayout(_currentLayout.get());
_camCtrl.applyLayoutFraming();
```

## PlayerTileSystem

Tracks which tile each player is on and dispatches movement events:

```cpp
_tileSystem.onPlayerAdded(player, _entities);
_tileSystem.onPlayerMoved(id, x, y, orientation, _entities, _clock);
_tileSystem.onPlayerRemoved(id);
_tileSystem.repositionAll(_entities, *_layout);  // after layout switch
```

Movement events emitted to `EntityManager`:
- `EntityMoveToEvent{id, targetPos, duration}` → picked up by `MovementBehavior`
- `EntityRotateToEvent{id, targetYaw, duration}` → picked up by `RotationBehavior`

Duration comes from `AnimationClock`:

```cpp
_clock.setTimeUnit(tu);          // updated from sgt protocol message
_clock.moveDuration()            // seconds per tile move
_clock.incantationDuration()     // seconds for incantation animation
```

## Default keybindings

| Key | Action |
|---|---|
| W / ↑ | Camera pan forward |
| S / ↓ | Camera pan backward |
| A / ← | Camera pan left |
| D / → | Camera pan right |
| Mouse drag | Orbit (yaw/pitch) |
| Scroll | Zoom (distance) |
| Space | Toggle orbit center ↔ free FPS |
| F | Follow toggle (single: follow/unfollow; double-tap: first-person) |
| G | Cycle map layout (Grid ↔ Torus) |
| T | Toggle tile shading |

All bindings are remappable via `SettingsPanel` → `KeybindingsSection`, which calls `InputManager::rebindAction`.
