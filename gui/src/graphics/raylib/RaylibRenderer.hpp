#pragma once
#include "graphic/IRenderer.hpp"
#include <raylib.h>
#include <unordered_map>

namespace graphic::raylib {

class RaylibRenderer : public IRenderer {
public:
    RaylibRenderer() = default;
    ~RaylibRenderer() override;

    bool        supports(RendererCap cap) const override;

    void        clear(Color4b color) override;
    Vector2f    getViewportSize() const override;

    void        setCamera(const CameraState& cam) override;
    CameraState getCamera() const override;

    void begin3D() override;
    void end3D()   override;
    void drawMesh(const MeshDrawParams& params) override;
    void drawGrid(int slices, float spacing)    override;

    void begin2D() override;
    void end2D()   override;
    void drawRect   (Rectangle2D rect, const ShapeStyle& style) override;
    void drawCircle (Vector2f center, float radius, const ShapeStyle& style) override;
    void drawLine   (Vector2f from, Vector2f to, float width, Color4b color) override;
    void drawSprite (const SpriteDrawParams& params) override;
    void drawText   (const std::string& text, Vector2f pos, const TextStyle& style) override;
    Vector2f measureText(const std::string& text, const TextStyle& style) override;

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
        MeshHandle mesh, Vector3f meshPosition) override;

private:
    ::Camera3D  _currentCamera{};
    CameraState _cameraState{};
    bool        _in3D = false;

    std::unordered_map<MeshHandle,    ::Model>    _models;
    std::unordered_map<TextureHandle, ::Texture2D> _textures;
    std::unordered_map<FontHandle,    ::Font>      _fonts;

    MeshHandle    _meshIdCounter    = {1};
    TextureHandle _textureIdCounter = {1};
    FontHandle    _fontIdCounter    = {1};

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
