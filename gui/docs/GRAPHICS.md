# Graphics System

## Abstraction layer

The graphics system uses four interfaces, all implemented by Raylib backends:

| Interface | Raylib impl | Purpose |
|---|---|---|
| `IWindowContext` | `RaylibWindow` | Window lifecycle, event polling, frame timing |
| `IRenderer` | `RaylibRenderer` | Drawing (3D/2D), resource loading (models, textures, fonts, shaders) |
| `IMeshFactory` | `RaylibMeshFactory` | Procedural mesh generation |
| `ITextureLoader` | `RaylibTextureLoader` | Texture loading from file or memory |
| `IFontLoader` | `RaylibFontLoader` | Font loading from file or memory |

`GraphicsContext` owns all five and exposes a single `pollAndDispatch()` / `beginFrame()` / `endFrame()` facade to `GameEngine`.

## Renderer

`IRenderer` is the main drawing surface. It is passed into entity behaviors via `RenderEvent`.

### Resource loading

```cpp
ModelHandle   h = renderer.loadModel("assets/models/player.glb");
TextureHandle t = renderer.loadTexture("assets/textures/tile.png");
FontHandle    f = renderer.loadFont("assets/fonts/ui.ttf");
ShaderHandle  s = renderer.loadShader("assets/shaders/wave.vs", "assets/shaders/wave.fs");
```

Handles are opaque integers. The renderer owns all GPU resources and frees them on destruction.

### Draw calls

```cpp
renderer.begin3D(cameraState);
    renderer.drawModel(modelHandle, transformMatrix, tintColor);
    renderer.drawMesh(meshHandle, transformMatrix, tintColor);
    renderer.drawGrassField(grassFieldHandle, cameraState);
    renderer.drawSkybox(skyboxHandle);
renderer.end3D();

renderer.begin2D();
    renderer.drawText(text, position, fontSize, color);
    // HudContainerBehavior uses internal Raylib 2D calls
renderer.end2D();
```

### Per-mesh tinting (`ModelDrawableBehavior`)

Models can have per-submesh color overrides via `setMeshTints(vector<Color>)`. Used for player skin/team color highlighting.

## Mesh factory

```cpp
MeshHandle plane  = factory.createPlane(width, height);
MeshHandle cube   = factory.createCube(size);
MeshHandle sphere = factory.createSphere(radius, rings, slices);
MeshHandle torus  = factory.createTorus(innerR, outerR, sides, rings);
MeshHandle cyl    = factory.createCylinder(radius, height, slices);
```

Used by `MapGroundFactory`, `GrassBuilder`, `RitualCircleBehavior`, and layout mesh generation.

## Grass system

`GrassBuilder` generates a `GrassField` — a set of blade meshes with randomized transforms and colors:

```cpp
GrassParams params{
    .density   = 4,       // blades per tile
    .height    = 0.3f,
    .color     = {60, 140, 60, 255},
    .noiseAmp  = 0.1f,
};
GrassFieldHandle grass = GrassBuilder::build(params, meshFactory);
renderer.drawGrassField(grass, camera);  // drawn with distance culling
```

The renderer draws grass with a custom wind-sway shader and distance-based LOD fade.

## Skybox

```cpp
SkyboxHandle sky = renderer.loadSkybox("assets/skybox/");
renderer.drawSkybox(sky);  // drawn first, before 3D pass
```

## Shaders

| Shader | Used by | Effect |
|---|---|---|
| Wave shader | `GroundDrawableBehavior`, `WaveBroadcastBehavior` | Expanding ripple / tile pulse |
| Outline shader | `OutlineBehavior` | Colored edge highlight on selected entities |
| Grass shader | `RaylibRenderer` (grass draw) | Wind sway + distance fade |
| Skybox shader | `RaylibRenderer` (skybox draw) | Cubemap sampling |

Shaders are loaded once by `RaylibRenderer` at init and referenced by handle throughout the session.

## Frame structure

Each frame in `GraphicsContext`:

1. `pollAndDispatch()` — reads OS events from Raylib, converts to `event::WindowEvent`, dispatches to scene.
2. `beginFrame()` — starts Raylib draw context, clears background.
3. Scene renders:
   - `begin3D(camera)` → entities draw models/meshes → grass → `end3D()`
   - Skybox drawn at start of 3D pass
   - `begin2D()` → HUD entities render → `end2D()`
4. `endFrame()` — swaps buffers.

## Adding a new rendered object

1. Load the model/texture in the relevant factory's `init()`:
   ```cpp
   _modelHandle = renderer.loadModel("assets/models/thing.glb");
   ```
2. Use `EntityBuilder.drawable().model(renderer, _modelHandle, color, rot)`.
3. For procedural geometry: use `EntityBuilder.drawable().mesh(...)` with a handle from `IMeshFactory`.
4. For custom shaders: load in `RaylibRenderer::init()`, apply inside a custom drawable behavior's `draw()`.
