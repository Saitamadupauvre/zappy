#pragma once
#include "graphic/IRenderer.hpp"
#include <raylib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "graphics/raylib/RaylibShaderManager.hpp"

namespace graphic::raylib {

class RaylibRenderer : public IRenderer {
public:
    RaylibRenderer();
    ~RaylibRenderer() override;

    void        init() override;

    bool        supports(RendererCap cap) const override;

    void        clear(Color4b color) override;
    Vector2f    getViewportSize() const override;

    void        setCamera(const CameraState& cam) override;
    CameraState getCamera() const override;

    void begin3D() override;
    void end3D()   override;
    void drawMesh(const MeshDrawParams& params) override;
    void drawModel(const ModelDrawParams& params) override;
    void drawGrid(int slices, float spacing)    override;

    void begin2D() override;
    void end2D()   override;
    void beginScissor(int x, int y, int width, int height) override;
    void endScissor() override;
    void drawRect   (Rectangle2D rect, const ShapeStyle& style) override;
    void drawCircle (Vector2f center, float radius, const ShapeStyle& style) override;
    void drawLine   (Vector2f from, Vector2f to, float width, Color4b color) override;
    void drawSprite (const SpriteDrawParams& params) override;
    void drawText   (const std::string& text, Vector2f pos, const TextStyle& style) override;
    Vector2f measureText(const std::string& text, const TextStyle& style) override;
    void drawOutline(const MeshDrawParams& params, float thickness, Color4b color) override;

    GrassFieldHandle uploadGrassField(const VertexData& bladeMesh,
        const std::vector<Matrix4x4>& transforms,
        const std::vector<Color4b>& colors) override;
    void drawGrassField(GrassFieldHandle field, const GrassDrawParams& params) override;
    void unloadGrassField(GrassFieldHandle field) override;

    void drawWave(const WaveDrawParams& params) override;
    void drawRitual(const RitualDrawParams& params) override;
    void drawLine3D(Vector3f from, Vector3f to, Color4b color) override;
    void drawSphere3D(Vector3f center, float radius, Color4b color) override;

    SkyboxHandle uploadSkybox() override;
    void         drawSkybox(SkyboxHandle handle, float time) override;
    void         unloadSkybox(SkyboxHandle handle) override;
    void         setSkyboxShader(SkyboxHandle handle, const std::string& shaderName) override;

    ModelHandle loadModel(const std::string& path) override;
    MeshHandle  meshFromModel(ModelHandle model) override;
    int  modelAnimationCount(ModelHandle model) const override;
    int  modelAnimationFrameCount(ModelHandle model, int animIndex) const override;
    void updateModelAnimation(ModelHandle model, int animIndex, float frame) override;
    void unloadModel(ModelHandle model) override;

    MeshHandle    uploadMesh   (const VertexData& data)                    override;
    TextureHandle uploadTexture(const TextureData& data)                   override;
    FontHandle    uploadFont   (const FontData& data) override;
    void unloadMesh   (MeshHandle    mesh)    override;
    void unloadTexture(TextureHandle texture) override;
    void unloadFont   (FontHandle    font)    override;

    Vector2f worldToScreen   (Vector3f worldPos)  const override;
    Vector3f screenToWorldRay(Vector2f screenPos) const override;

    RayCollisionData checkRayMeshCollision(
        Vector3f rayOrigin, Vector3f rayDirection,
        MeshHandle mesh, Vector3f meshPosition, Vector3f meshScale = {1.f, 1.f, 1.f}) override;
    void setDepthTest(bool enabled) override;

private:
    ::Camera3D  _currentCamera{};
    CameraState _cameraState{};
    bool        _in3D = false;

    struct ModelEntry {
        ::Model           model{};
        ::ModelAnimation* anims    = nullptr;
        int               animCount = 0;
    };

    struct GrassFieldEntry {
        ::Mesh                  mesh{};
        ::Material              material{};
        std::vector<::Matrix>   transforms;     // column-major, ready for raylib
        unsigned int            colorVboId = 0;  // per-instance vec4 tint buffer
        int                     instanceColorLoc = -1;
    };

    std::unordered_map<MeshHandle,    ::Model>    _models;
    std::unordered_map<TextureHandle, ::Texture2D> _textures;
    std::unordered_map<FontHandle,    ::Font>      _fonts;
    std::unordered_map<ModelHandle,   ModelEntry>  _loadedModels;
    // MeshHandles in _models that are non-owning views over a loaded model;
    // their GPU resources belong to _loadedModels and must not be unloaded here.
    std::unordered_set<MeshHandle>                 _meshViews;
    std::unordered_map<GrassFieldHandle, GrassFieldEntry> _grassFields;

    struct SkyboxEntry {
        ::Model model{};
        std::string currentShaderName{"skybox"};
    };
    std::unordered_map<SkyboxHandle, SkyboxEntry> _skyboxes;
    SkyboxHandle _skyboxIdCounter = {1};

    MeshHandle      _meshIdCounter    = {1};
    TextureHandle   _textureIdCounter = {1};
    FontHandle      _fontIdCounter    = {1};
    ModelHandle     _modelIdCounter   = {1};
    GrassFieldHandle _grassIdCounter  = {1};

    RaylibShaderManager _shaderManager;

    std::vector<::Shader> _shaderDefaultsBuf;

    // Helpers
    ::Color toRayColor(Color4b c) const { return {c.r, c.g, c.b, c.a}; }
    ::Color toRayColorAlpha(Color4b c, float opacity) const {
        return {c.r, c.g, c.b, static_cast<unsigned char>(c.a * opacity)};
    }
    ::Rectangle toRayRect(Rectangle2D r, bool pixelSpace = true) const;
    void applyFillDraw (Rectangle2D rect, const ShapeStyle& style) const;
    void applyStrokeDraw(Rectangle2D rect, const ShapeStyle& style) const;
    void applyEffects  (Rectangle2D rect, const ShapeStyle& style) const;
};

} // namespace graphic::raylib
