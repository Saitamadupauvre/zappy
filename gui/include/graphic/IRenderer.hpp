#pragma once

#include "graphic/Types.hpp"
#include <string>
#include <vector>

namespace graphic {

class IRenderer {
    public:
        IRenderer() = default;
        virtual ~IRenderer() = default;

        virtual void     init() = 0;
        virtual bool     supports(RendererCap cap) const = 0;

        virtual void     clear(Color4b color) = 0;
        virtual Vector2f getViewportSize() const = 0;

        virtual void         setCamera(const CameraState& cam) = 0;
        virtual CameraState  getCamera() const = 0;

        virtual void begin3D() = 0;
        virtual void end3D()   = 0;
        virtual void drawMesh(const MeshDrawParams& params) = 0;
        virtual void drawModel(const ModelDrawParams& params) = 0;
        virtual void drawGrid(int slices, float spacing)    = 0;

        virtual void begin2D() = 0;
        virtual void end2D()   = 0;
        virtual void beginScissor(int x, int y, int width, int height) = 0;
        virtual void endScissor() = 0;
        virtual void drawRect   (Rectangle2D rect, const ShapeStyle& style) = 0;
        virtual void drawCircle (Vector2f center, float radius, const ShapeStyle& style) = 0;
        virtual void drawLine   (Vector2f from, Vector2f to, float width, Color4b color) = 0;
        virtual void drawSprite (const SpriteDrawParams& params) = 0;
        virtual void drawText   (const std::string& text, Vector2f pos, const TextStyle& style) = 0;
        virtual Vector2f measureText(const std::string& text, const TextStyle& style) = 0;
        virtual void drawOutline(const MeshDrawParams& params, float thickness, Color4b color) = 0;

        // Instanced grass: scatter `transforms.size()` blades (one model matrix +
        // tint each) sharing a single `bladeMesh`, drawn with the wind shader.
        // Works for any source surface — the caller scatters the instances.
        virtual GrassFieldHandle uploadGrassField(const VertexData& bladeMesh,
            const std::vector<Matrix4x4>& transforms,
            const std::vector<Color4b>& colors) = 0;
        virtual void drawGrassField(GrassFieldHandle field, const GrassDrawParams& params) = 0;
        virtual void unloadGrassField(GrassFieldHandle field) = 0;

        virtual void drawWave(const WaveDrawParams& params) = 0;
        virtual void drawRitual(const RitualDrawParams& params) = 0;

        // 3D glow primitives — drawn with additive blending so they bloom on top of the scene.
        // Call only inside begin3D() / end3D().
        virtual void drawLine3D(Vector3f from, Vector3f to, Color4b color) = 0;
        virtual void drawSphere3D(Vector3f center, float radius, Color4b color) = 0;

        virtual SkyboxHandle uploadSkybox() = 0;
        virtual void         drawSkybox(SkyboxHandle handle, float time) = 0;
        virtual void         unloadSkybox(SkyboxHandle handle) = 0;
        virtual void         setSkyboxShader(SkyboxHandle handle, const std::string& shaderName) = 0;

        virtual ModelHandle loadModel(const std::string& path) = 0;
        // Registers a non-owning mesh view over a loaded model so mesh-based
        // systems (picking, outline) can treat the model like a regular mesh.
        virtual MeshHandle meshFromModel(ModelHandle model) = 0;
        virtual int  modelAnimationCount(ModelHandle model) const = 0;
        virtual int  modelAnimationFrameCount(ModelHandle model, int animIndex) const = 0;
        virtual void updateModelAnimation(ModelHandle model, int animIndex, float frame) = 0;
        virtual void unloadModel(ModelHandle model) = 0;

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
            MeshHandle mesh, Vector3f meshPosition, Vector3f meshScale = {1.f, 1.f, 1.f}) = 0;

        virtual void setDepthTest(bool enabled) = 0;
};

} // namespace graphic
