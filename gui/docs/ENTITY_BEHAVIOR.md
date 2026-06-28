# Entity & Behavior System

## Concepts

Every visible or interactive object is a `graphic::Entity` — a thin container of an ID, a type tag string, and a list of `shared_ptr<IBehavior>`. Behaviors are where all logic lives.

```
graphic::Entity (id, type)
  └── IBehavior::onAttach(entity)      — called once on addBehavior<T>()
  └── IBehavior::onUpdate(entity, dt)  — called each frame
  └── IBehavior::onEvent(entity, ev)   — called for each dispatched event
```

Entities are stored in `EntityManager` (typedef `HudManager` for the 2D layer). The manager owns entity lifetimes and iterates them for update/event dispatch.

## Creating an entity — EntityBuilder

Never construct entities manually. Always use `EntityBuilder`:

```cpp
#include "scene/builder/EntityBuilder.hpp"

EntityBuilder(entityManager, id, "my_type")
    .transform().position({x, 0.f, z})
    .transform().scale(1.0f)
    .drawable().model(renderer, modelHandle, color, rotation)
    .interaction().onClick([](graphic::Entity& e){ /* ... */ })
    .interaction().selectableOutline()
    .build();
```

`.build()` returns an `EntityPtr`. You can immediately call `.getBehavior<T>()` on it.

### Full builder chain reference

```cpp
EntityBuilder(em, id, "tag")
    // Transform
    .transform().position({x, y, z})
    .transform().scale(s)
    .transform().rotation({rx, ry, rz})

    // Drawables (pick one)
    .drawable().model(renderer, handle, color, rotationDeg)  // 3D model
    .drawable().mesh(renderer, meshHandle, color)             // procedural mesh
    .drawable().text("label", fontSize, color)                // world-space text

    // Interaction
    .interaction().onClick(fn)            // fn: void(graphic::Entity&)
    .interaction().hoverable()
    .interaction().selectable()           // emits EntitySelectedEvent on click
    .interaction().selectableOutline()    // selectable + outline shader

    // Player-specific
    .player().state(playerState)
    .player().broadcast()
    .player().animation(animData)
    .player().levelModel(levelModels)

    // HUD (see HUD.md for full reference)
    .hud().container(providerPtr)
    .hud().layout(Type::Vertical, 8.f)
    .hud().anchor(graphic::Anchor::TopRight)
    .hud().anchorOffset({10.f, 175.f})
    .hud().background(true, fillColor, borderColor)
    .hud().boxSize({300.f, 400.f})
    .hud().title("Panel Title", 14.f)
    .hud().isWorldSpaceTag(true)
    .hud().hidden()

    .build();
```

## Writing a custom behavior

```cpp
#include "behavior/ABehavior.hpp"  // no-op default implementations

class MyBehavior : public behavior::ABehavior {
public:
    void onAttach(graphic::Entity& owner) override {
        // cache owner ID, set initial state
    }

    void onUpdate(graphic::Entity& owner, float dt) override {
        _elapsed += dt;
    }

    void onEvent(graphic::Entity& owner, const event::Event& ev) override {
        event::on(ev,
            [&](const event::EntityMoveToEvent& e) {
                if (e.entityId == owner.getID()) { /* respond */ }
            }
        );
    }

private:
    float _elapsed = 0.f;
};
```

Attach and retrieve:

```cpp
e->addBehavior<MyBehavior>(constructorArg1, constructorArg2);
auto b = e->getBehavior<MyBehavior>(); // shared_ptr<MyBehavior> or nullptr
if (b) b->doSomething();
```

## Writing a drawable behavior

Extend `ADrawable` (which extends `ABehavior`) and implement `draw()`:

```cpp
class MyDrawable : public behavior::ADrawable {
public:
    void draw(graphic::Entity& owner, graphic::IRenderer& renderer) override {
        auto* tf = owner.getBehavior<behavior::TransformBehavior>().get();
        if (!tf) return;
        renderer.drawMesh(_meshHandle, tf->getMatrix(), _color);
    }
private:
    graphic::MeshHandle _meshHandle;
    graphic::Color4b    _color;
};
```

`ADrawable::onEvent` already dispatches `Draw3DEvent` / `Draw2DEvent` to `draw()` — you only need to implement `draw()`.

## Existing behavior catalogue

### Drawable behaviors

| Behavior | Purpose |
|---|---|
| `ModelDrawableBehavior` | Renders a `.glb` 3D model with optional per-mesh tints |
| `MeshDrawableBehavior` | Renders a procedural mesh |
| `GroundDrawableBehavior` | Ground tile with wave shader + grass field |
| `TextDrawableBehavior` | World-space text label |
| `HudContainerBehavior` | 2D HUD panel — see [HUD.md](HUD.md) |

### Transform & movement behaviors

| Behavior | Purpose |
|---|---|
| `TransformBehavior` | Position, rotation, scale, custom rotation matrix |
| `MovementBehavior` | Smooth position interpolation (listens to `EntityMoveToEvent`) |
| `RotationBehavior` | Smooth yaw + up/forward interpolation (listens to `EntityRotateToEvent`) |
| `PlayerMovementBehavior` | Player-specific movement with tile-wrap awareness |
| `PlayerOrientationBehavior` | Player facing direction sync |

### Interaction behaviors

| Behavior | Purpose |
|---|---|
| `ClickableBehavior` | Fires `onClick` callback on `ClickEvent` |
| `HoverableBehavior` | Calls `onEnter`/`onLeave` on `HoverEvent` |
| `SelectableBehavior` | Emits `EntitySelectedEvent` on click |
| `OutlineBehavior` | Outline shader on select/hover |

### Player behaviors

| Behavior | Purpose |
|---|---|
| `PlayerBehavior` | Immutable player state snapshot (id, pos, level, team, inventory) |
| `PlayerAnimationBehavior` | Skeletal animation state machine (Idle, Run, Take, Dance) |
| `PlayerLevelModelBehavior` | Swaps model + mesh tints on level-up |
| `BroadcastBehavior` | Speech bubble timer on `PlayerBroadcastEvent` |

### Effect behaviors

| Behavior | Purpose |
|---|---|
| `WaveBroadcastBehavior` | Expanding ring wave on broadcast |
| `RitualCircleBehavior` | Rotating incantation circle projected onto ground |
| `IncantationTileBehavior` | Tile pulsation during incantation |
| `ExplosionBehavior` | Multi-rocket firework explosion on incantation success |
| `EggBehavior` | Egg pulsation + removal on hatch/death |

### World-space tag behaviors

| Behavior | Purpose |
|---|---|
| `TagBehavior` | Moves a HUD entity to follow a 3D entity's screen position |
| `RectTransformBehavior` | 2D UI rectangle transform for HUD hit-testing |

## Effect behaviors — implementation notes

### ExplosionBehavior

Spawned on `IncantationEndEvent` (success only). `NUM_FW` instances per incantation, one per firework color.

**Lifecycle phases:** `Waiting` → `Rising` (rockets travel to burst center) → `Bursting` (sparks fan out with gravity + drag).

**Performance budget:** Total live sparks = `NUM_FW × N_ROCKETS × N_SPARKS`. Keep under ~150. Tuned values: `NUM_FW=4`, `N_ROCKETS=3`, `N_SPARKS=12` → 144 max.

**Hot-path rule:** `onUpdate` precomputes `velDir`, `trailLen`, `tailColor` per spark. Never call `sqrt` or recompute derived values inside the render path.

### WaveBroadcastBehavior

Spawned on `PlayerBroadcastEvent`. 64 reusable slots (`WAVE_BASE_ID + slot % 64`); oldest evicted when full. Uses a wave shader for the expanding ring.

### RitualCircleBehavior

Spawned on `IncantationStartEvent`. One per incantating tile (`RITUAL_BASE_ID + x*10000 + y`). Projects a rotating mesh onto the tile surface; works correctly on both Grid and Torus layouts.

## Adding a new entity type

1. Create a factory in `src/scene/factory/` (follow `PlayerEntityFactory`).
2. In `init()`, load models/textures via the renderer.
3. In `spawn()`, use `EntityBuilder` with appropriate behaviors.
4. Pick a non-colliding ID range (see [ARCHITECTURE.md](ARCHITECTURE.md#entity-id-allocation)).
5. Instantiate the factory in `WorldScene`; call `spawn()` from the relevant event handler.

## Removing an entity

```cpp
_entities.removeEntity(id);  // 3D entities
_hud.removeEntity(id);       // HUD entities
```

Behaviors are destroyed with the entity. No manual cleanup needed for `shared_ptr` members.
