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

    // Drawables (pick one)
    .drawable().model(renderer, handle, color, rotationDeg)  // 3D model
    .drawable().mesh(renderer, meshHandle, color)             // procedural mesh
    .drawable().text("label", fontSize, color)                // world-space text

    // Interaction
    .interaction().onClick(fn)           // fn: void(graphic::Entity&)
    .interaction().hoverable()
    .interaction().selectableOutline()   // shows outline shader on SelectEvent

    // HUD (see HUD.md for full reference)
    .hud().container(providerPtr)
    .hud().layout(Type::Vertical, 8.f)
    .hud().anchor(graphic::Anchor::TopRight)
    .hud().anchorOffset({10.f, 175.f})
    .hud().background(true, fillColor, borderColor)
    .hud().boxSize({300.f, 400.f})
    .hud().title("Panel Title", 14.f)
    .hud().hidden()

    .build();
```

## Writing a custom behavior

```cpp
#include "behavior/ABehavior.hpp"  // no-op default implementations

class MyBehavior : public behavior::ABehavior {
public:
    // Called once when the behavior is attached to an entity.
    void onAttach(graphic::Entity& owner) override {
        // cache owner ID, set initial state
    }

    // Called every frame.
    void onUpdate(graphic::Entity& owner, float dt) override {
        _elapsed += dt;
    }

    // Called for every event dispatched to this entity.
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

Then attach it:

```cpp
auto e = entityManager.getEntity(id);
e->addBehavior<MyBehavior>();
// or
e->addBehavior<MyBehavior>(constructorArg1, constructorArg2);
```

Retrieve it later:

```cpp
auto b = e->getBehavior<MyBehavior>(); // returns shared_ptr<MyBehavior> or nullptr
if (b) b->doSomething();
```

## Existing behavior catalogue

### Drawable behaviors

| Behavior | File | Purpose |
|---|---|---|
| `ModelDrawableBehavior` | `src/behavior/drawable/model/` | Renders a .glb 3D model |
| `MeshDrawableBehavior` | `src/behavior/drawable/mesh/` | Renders a procedural mesh |
| `GroundDrawableBehavior` | `src/behavior/drawable/mesh/` | Ground tile with wave shader |
| `TextDrawableBehavior` | `src/behavior/drawable/tag/` | World-space text label |
| `HudContainerBehavior` | `src/behavior/hud/` | 2D HUD panel (see [HUD.md](HUD.md)) |

### Movement behaviors

| Behavior | File | Purpose |
|---|---|---|
| `MovementBehavior` | `src/behavior/movement/` | Smooth position interpolation |
| `RotationBehavior` | `src/behavior/movement/` | Smooth yaw rotation interpolation |
| `PlayerMovementBehavior` | `src/behavior/player/` | Player-specific movement with tile-wrap awareness |
| `PlayerOrientationBehavior` | `src/behavior/player/` | Player facing direction sync |

### Interaction behaviors

| Behavior | File | Purpose |
|---|---|---|
| `ClickableBehavior` | `src/behavior/clickable/` | Fires `onClick` on `ClickEvent` |
| `HoverableBehavior` | `src/behavior/hoverable/` | Tracks hover state |
| `SelectableBehavior` | `src/behavior/selectable/` | Emits `EntitySelectedEvent` on click |
| `OutlineBehavior` | `src/behavior/outline/` | Outline shader on select |

### Effect behaviors

| Behavior | File | Purpose |
|---|---|---|
| `BroadcastBehavior` | `src/behavior/player/` | Renders broadcast speech bubble |
| `WaveBroadcastBehavior` | `src/behavior/wave/` | Expanding ring wave effect |
| `RitualCircleBehavior` | `src/behavior/incantation/` | Incantation ritual circle |
| `ExplosionBehavior` | `src/behavior/incantation/` | Post-incantation explosion |
| `EggBehavior` | `src/behavior/egg/` | Egg hatching animation |
| `PlayerAnimationBehavior` | `src/behavior/animation/` | Model animation state machine |
| `TagBehavior` | `src/behavior/tag/` | Positions a HUD entity over a world entity |

## Effect behaviors — implementation notes

### ExplosionBehavior

Spawned by `WorldScene::spawnExplosion` on `IncantationEndEvent` (success only). Multiple behavior instances are created per incantation — one per firework color offset (`NUM_FW` in `WorldScene`).

**Lifecycle phases:** `Waiting` (start delay) → `Rising` (rockets travel toward burst center) → `Bursting` (sparks fan out with gravity + drag).

**Performance budget:** Keep total live sparks (`NUM_FW × N_ROCKETS × N_SPARKS`) under ~150. Current tuned values: `NUM_FW=4`, `N_ROCKETS=3`, `N_SPARKS=12` → 144 sparks max.

**Hot-path rule:** `onUpdate` precomputes `velDir` (normalized velocity), `trailLen`, and `tailColor` for each spark. `renderBurst` uses these cached values — never call `sqrt` or recompute per-frame derived values inside the render path.

**Spark draw calls per frame:** 2× `drawLine3D` + 1× `drawSphere3D` (only when `life > 0.25`).

### WaveBroadcastBehavior

Spawned by `WorldScene::spawnWave` on `PlayerBroadcastEvent`. Reuses 64 slots (`WAVE_BASE_ID + slot % 64`); oldest wave is evicted when all slots are full.

### RitualCircleBehavior

Spawned by `WorldScene::spawnRituals` on `IncantationStartEvent`. One entity per tile (`RITUAL_BASE_ID + x*10000 + y`). Removed implicitly when the incantation ends (tile ID is reused on next incantation at the same tile).

## Adding a new entity type

1. Create a factory class in `src/scene/factory/` (follow `PlayerEntityFactory` as a template).
2. In `init()`, load models/textures via the renderer.
3. In `spawn()`, use `EntityBuilder` to create the entity with appropriate behaviors.
4. Pick a non-colliding ID range (see [ARCHITECTURE.md](ARCHITECTURE.md#entity-id-allocation)).
5. Instantiate the factory in `WorldScene` and call `spawn()` from the relevant event handler (`onPlayerAdded`, etc.).

## Removing an entity

```cpp
_entities.removeEntity(id);
// or for HUD entities:
_hud.removeEntity(id);
```

Behaviors are destroyed with the entity. No manual cleanup needed for shared_ptr members.
