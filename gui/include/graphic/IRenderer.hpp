#pragma once

#include "graphic/Types.hpp"
#include <string>

namespace graphic {

class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual bool     supports(RendererCap cap) const = 0;

        virtual void     clear(Color4b color) = 0;
        virtual Vector2f getViewportSize() const = 0;

        virtual void         setCamera(const CameraState& cam) = 0;
        virtual CameraState  getCamera() const = 0;

        virtual void begin3D() = 0;
        virtual void end3D()   = 0;
        virtual void drawMesh(const MeshDrawParams& params) = 0;
        virtual void drawGrid(int slices, float spacing)    = 0;

        virtual void begin2D() = 0;
        virtual void end2D()   = 0;
        virtual void drawRect   (Rectangle2D rect, const ShapeStyle& style) = 0;
        virtual void drawCircle (Vector2f center, float radius, const ShapeStyle& style) = 0;
        virtual void drawLine   (Vector2f from, Vector2f to, float width, Color4b color) = 0;
        virtual void drawSprite (const SpriteDrawParams& params) = 0;
        virtual void drawText   (const std::string& text, Vector2f pos, const TextStyle& style) = 0;
        virtual Vector2f measureText(const std::string& text, const TextStyle& style) = 0;

        virtual MeshHandle    uploadMesh   (const VertexData& data)  = 0;
        virtual TextureHandle uploadTexture(const TextureData& data) = 0;
        virtual FontHandle    uploadFont   (const FontData& data)    = 0;
        virtual void unloadMesh   (MeshHandle    mesh)    = 0;
        virtual void unloadTexture(TextureHandle texture) = 0;
        virtual void unloadFont   (FontHandle    font)    = 0;

        virtual Vector2f worldToScreen    (Vector3f worldPos)   const = 0;
        virtual Vector3f screenToWorldRay (Vector2f screenPos)  const = 0;

        virtual RayCollisionData checkRayMeshCollision(
            Vector3f rayOrigin, Vector3f rayDirection,
            MeshHandle mesh, Vector3f meshPosition) = 0;
};

} // namespace graphic
