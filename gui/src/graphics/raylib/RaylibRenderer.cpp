#include "RaylibRenderer.hpp"
#include "util/Overloaded.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace graphic::raylib {

RaylibRenderer::~RaylibRenderer()
{
    for (auto& [id, model]   : _models)   UnloadModel(model);
    for (auto& [id, texture] : _textures) UnloadTexture(texture);
    for (auto& [id, font]    : _fonts)    UnloadFont(font);
}

bool RaylibRenderer::supports(RendererCap cap) const
{
    switch (cap) {
        case RendererCap::RoundedCorners: return true;
        case RendererCap::DropShadow:     return true;
        case RendererCap::TextOutline:    return true;
        case RendererCap::Instancing:     return false;
        case RendererCap::CustomShaders:  return false;
    }
    return false;
}

void RaylibRenderer::clear(Color4b color)
{
    ClearBackground(toRayColor(color));
}

Vector2f RaylibRenderer::getViewportSize() const
{
    return { static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) };
}

void RaylibRenderer::setCamera(const CameraState& cam)
{
    _cameraState = cam;
    _currentCamera.position   = { cam.position.x, cam.position.y, cam.position.z };
    _currentCamera.target     = { cam.target.x, cam.target.y, cam.target.z };
    _currentCamera.up         = { cam.up.x, cam.up.y, cam.up.z };
    _currentCamera.fovy       = cam.fovDegrees;
    _currentCamera.projection = CAMERA_PERSPECTIVE;
}

CameraState RaylibRenderer::getCamera() const
{
    return _cameraState;
}

void RaylibRenderer::begin3D()
{
    _in3D = true;
    BeginMode3D(_currentCamera);
}

void RaylibRenderer::end3D()
{
    _in3D = false;
    EndMode3D();
}

void RaylibRenderer::drawMesh(const MeshDrawParams& p)
{
    auto it = _models.find(p.mesh);
    if (it == _models.end()) return;

    ::Model model = it->second;
    if (p.texture.id != 0) {
        auto texIt = _textures.find(p.texture);
        if (texIt != _textures.end())
            model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texIt->second;
    }

    const float* m = p.transform.m;
    model.transform = ::Matrix{
        m[0],  m[1],  m[2],  m[3],
        m[4],  m[5],  m[6],  m[7],
        m[8],  m[9],  m[10], m[11],
        m[12], m[13], m[14], m[15]
    };

    ::Vector3 origin = {0.f, 0.f, 0.f};
    ::Color   col    = toRayColor(p.tint);

    if (p.wireframe)
        DrawModelWires(model, origin, 1.f, col);
    else
        DrawModel(model, origin, 1.f, col);
}

void RaylibRenderer::drawGrid(int slices, float spacing)
{
    DrawGrid(slices, spacing);
}

void RaylibRenderer::begin2D() { /* Raylib 2D draws anywhere inside BeginDrawing() */ }
void RaylibRenderer::end2D()   { }

::Rectangle RaylibRenderer::toRayRect(Rectangle2D r, bool /*pixelSpace*/) const
{
    return { r.position.x, r.position.y, r.size.x, r.size.y };
}

static ::Color blendAlpha(::Color c, float opacity)
{
    c.a = static_cast<unsigned char>(c.a * opacity);
    return c;
}

static ::Color resolveFillColor(const Fill& fill, float opacity)
{
    return std::visit(overloaded{
        [&](const SolidFill& f)   -> ::Color { return blendAlpha({f.color.r, f.color.g, f.color.b, f.color.a}, opacity); },
        [&](const TextureFill& f) -> ::Color { return blendAlpha({f.tint.r,  f.tint.g,  f.tint.b,  f.tint.a},  opacity); },
    }, fill);
}

void RaylibRenderer::applyEffects(Rectangle2D rect, const ShapeStyle& style) const
{
    for (const auto& fx : style.effects) {
        std::visit(overloaded{
            [&](const ShadowEffect& s) {
                Rectangle2D shadowRect = {
                    { rect.position.x + s.offset.x, rect.position.y + s.offset.y },
                    rect.size
                };
                ::Color sc = {s.color.r, s.color.g, s.color.b, s.color.a};
                if (style.cornerRadius > 0.f)
                    DrawRectangleRounded(toRayRect(shadowRect), style.cornerRadius / (rect.size.x * 0.5f), 8, sc);
                else
                    DrawRectangleRec(toRayRect(shadowRect), sc);
            }
        }, fx);
    }
}

void RaylibRenderer::applyFillDraw(Rectangle2D rect, const ShapeStyle& style) const
{
    if (!style.fill) return;
    ::Color col = resolveFillColor(*style.fill, style.opacity);

    if (std::holds_alternative<TextureFill>(*style.fill)) {
        const auto& tf = std::get<TextureFill>(*style.fill);
        auto texIt = _textures.find(tf.texture);
        if (texIt != _textures.end()) {
            ::Texture2D tex = texIt->second;
            ::Rectangle src = {
                tf.uvCrop.position.x * tex.width,
                tf.uvCrop.position.y * tex.height,
                tf.uvCrop.size.x     * tex.width,
                tf.uvCrop.size.y     * tex.height
            };
            ::Rectangle dst = toRayRect(rect);
            ::Vector2 origin = {0, 0};
            DrawTexturePro(tex, src, dst, origin, 0.f, col);
            return;
        }
    }

    if (style.cornerRadius > 0.f) {
        float roundness = style.cornerRadius / (std::min(rect.size.x, rect.size.y) * 0.5f);
        DrawRectangleRounded(toRayRect(rect), roundness, 8, col);
    } else {
        DrawRectangleRec(toRayRect(rect), col);
    }
}

void RaylibRenderer::applyStrokeDraw(Rectangle2D rect, const ShapeStyle& style) const
{
    if (!style.stroke) return;
    const auto& stroke = *style.stroke;
    ::Color col = resolveFillColor(stroke.paint, style.opacity);
    if (style.cornerRadius > 0.f) {
        float roundness = style.cornerRadius / (std::min(rect.size.x, rect.size.y) * 0.5f);
        DrawRectangleRoundedLines(toRayRect(rect), roundness, 8, col);
    } else {
        DrawRectangleLinesEx(toRayRect(rect), stroke.width, col);
    }
}

void RaylibRenderer::drawRect(Rectangle2D rect, const ShapeStyle& style)
{
    applyEffects   (rect, style);
    applyFillDraw  (rect, style);
    applyStrokeDraw(rect, style);
}

void RaylibRenderer::drawCircle(Vector2f center, float radius, const ShapeStyle& style)
{
    for (const auto& fx : style.effects) {
        std::visit(overloaded{
            [&](const ShadowEffect& s) {
                ::Color sc = {s.color.r, s.color.g, s.color.b, s.color.a};
                DrawCircleV({center.x + s.offset.x, center.y + s.offset.y}, radius, sc);
            }
        }, fx);
    }

    if (style.fill) {
        ::Color col = resolveFillColor(*style.fill, style.opacity);
        DrawCircleV({center.x, center.y}, radius, col);
    }
    if (style.stroke) {
        ::Color col = resolveFillColor(style.stroke->paint, style.opacity);
        DrawCircleLinesV({center.x, center.y}, radius, col);
    }
}

void RaylibRenderer::drawLine(Vector2f from, Vector2f to, float width, Color4b color)
{
    DrawLineEx({from.x, from.y}, {to.x, to.y}, width, toRayColor(color));
}

void RaylibRenderer::drawSprite(const SpriteDrawParams& p)
{
    auto texIt = _textures.find(p.texture);
    if (texIt == _textures.end()) return;

    ::Texture2D tex = texIt->second;
    ::Rectangle src = {
        p.srcRect.position.x * tex.width,
        p.srcRect.position.y * tex.height,
        p.srcRect.size.x     * tex.width,
        p.srcRect.size.y     * tex.height
    };
    ::Rectangle dst = toRayRect(p.dstRect);
    ::Vector2   origin = {0, 0};
    ::Color     tint   = blendAlpha(toRayColor(p.tint), p.opacity);

    DrawTexturePro(tex, src, dst, origin, p.rotation, tint);
}

void RaylibRenderer::drawText(const std::string& text, Vector2f pos, const TextStyle& style)
{
    ::Font drawFont = GetFontDefault();
    if (style.font.id != 0) {
        auto it = _fonts.find(style.font);
        if (it != _fonts.end()) drawFont = it->second;
    }

    ::Color col = blendAlpha(toRayColor(style.color), style.opacity);

    if (style.shadow) {
        ::Color sc = blendAlpha(toRayColor(style.shadow->color), style.opacity);
        DrawTextEx(drawFont, text.c_str(),
            {pos.x + style.shadow->offset.x, pos.y + style.shadow->offset.y},
            static_cast<float>(style.size), 1.f, sc);
    }

    if (style.outline) {
        ::Color oc = resolveFillColor(style.outline->paint, style.opacity);
        float   ow = style.outline->width;
        for (float dx = -ow; dx <= ow; dx += ow) {
            for (float dy = -ow; dy <= ow; dy += ow) {
                if (dx == 0.f && dy == 0.f) continue;
                DrawTextEx(drawFont, text.c_str(), {pos.x + dx, pos.y + dy},
                    static_cast<float>(style.size), 1.f, oc);
            }
        }
    }

    DrawTextEx(drawFont, text.c_str(), {pos.x, pos.y}, static_cast<float>(style.size), 1.f, col);
}

Vector2f RaylibRenderer::measureText(const std::string& text, const TextStyle& style)
{
    ::Font drawFont = GetFontDefault();
    if (style.font.id != 0) {
        auto it = _fonts.find(style.font);
        if (it != _fonts.end()) drawFont = it->second;
    }
    ::Vector2 sz = MeasureTextEx(drawFont, text.c_str(), static_cast<float>(style.size), 1.f);
    return { sz.x, sz.y };
}

MeshHandle RaylibRenderer::uploadMesh(const VertexData& data)
{
    ::Mesh rayMesh{};
    rayMesh.vertexCount   = static_cast<int>(data.positions.size());
    rayMesh.triangleCount = static_cast<int>(data.indices.size() / 3);

    rayMesh.vertices = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 3 * sizeof(float)));
    for (int i = 0; i < rayMesh.vertexCount; ++i) {
        rayMesh.vertices[i*3]   = data.positions[i].x;
        rayMesh.vertices[i*3+1] = data.positions[i].y;
        rayMesh.vertices[i*3+2] = data.positions[i].z;
    }
    if (!data.texCoords.empty()) {
        rayMesh.texcoords = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 2 * sizeof(float)));
        for (int i = 0; i < rayMesh.vertexCount; ++i) {
            rayMesh.texcoords[i*2]   = data.texCoords[i].x;
            rayMesh.texcoords[i*2+1] = data.texCoords[i].y;
        }
    }
    if (!data.normals.empty()) {
        rayMesh.normals = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 3 * sizeof(float)));
        for (int i = 0; i < rayMesh.vertexCount; ++i) {
            rayMesh.normals[i*3]   = data.normals[i].x;
            rayMesh.normals[i*3+1] = data.normals[i].y;
            rayMesh.normals[i*3+2] = data.normals[i].z;
        }
    }
    rayMesh.indices = static_cast<unsigned short*>(MemAlloc(rayMesh.triangleCount * 3 * sizeof(unsigned short)));
    for (int i = 0; i < rayMesh.triangleCount * 3; ++i)
        rayMesh.indices[i] = data.indices[i];

    UploadMesh(&rayMesh, false);

    MeshHandle id = _meshIdCounter;
    _meshIdCounter.id++;
    _models[id] = LoadModelFromMesh(rayMesh);
    return id;
}

TextureHandle RaylibRenderer::uploadTexture(const TextureData& data)
{
    ::Image img{};
    img.data    = const_cast<void*>(static_cast<const void*>(data.pixels.data()));
    img.width   = data.width;
    img.height  = data.height;
    img.mipmaps = 1;
    img.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    TextureHandle id = _textureIdCounter;
    _textureIdCounter.id++;
    _textures[id] = LoadTextureFromImage(img);
    return id;
}

FontHandle RaylibRenderer::uploadFont(const FontData& data)
{
    ::Image atlasImg{};
    atlasImg.data    = const_cast<uint8_t*>(data.atlas.pixels.data());
    atlasImg.width   = data.atlas.width;
    atlasImg.height  = data.atlas.height;
    atlasImg.mipmaps = 1;
    atlasImg.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ::Font font{};
    font.baseSize   = data.size;
    font.glyphCount = static_cast<int>(data.glyphs.size());
    font.texture    = LoadTextureFromImage(atlasImg);
    font.recs       = static_cast<::Rectangle*>(MemAlloc(font.glyphCount * sizeof(::Rectangle)));
    font.glyphs     = static_cast<::GlyphInfo*>(MemAlloc(font.glyphCount * sizeof(::GlyphInfo)));

    float atlasW = static_cast<float>(data.atlas.width);
    float atlasH = static_cast<float>(data.atlas.height);
    for (int i = 0; i < font.glyphCount; ++i) {
        const auto& g = data.glyphs[i];
        font.recs[i] = {
            g.u0 * atlasW, g.v0 * atlasH,
            (g.u1 - g.u0) * atlasW, (g.v1 - g.v0) * atlasH
        };
        font.glyphs[i].value    = g.codepoint;
        font.glyphs[i].offsetX  = static_cast<int>(g.offsetX);
        font.glyphs[i].offsetY  = static_cast<int>(g.offsetY);
        font.glyphs[i].advanceX = static_cast<int>(g.advanceX);
        font.glyphs[i].image    = {};
    }

    FontHandle id = _fontIdCounter;
    _fontIdCounter.id++;
    _fonts[id] = font;
    return id;
}

void RaylibRenderer::unloadMesh(MeshHandle mesh)
{
    auto it = _models.find(mesh);
    if (it == _models.end()) return;
    UnloadModel(it->second);
    _models.erase(it);
}

void RaylibRenderer::unloadTexture(TextureHandle texture)
{
    auto it = _textures.find(texture);
    if (it == _textures.end()) return;
    UnloadTexture(it->second);
    _textures.erase(it);
}

void RaylibRenderer::unloadFont(FontHandle font)
{
    auto it = _fonts.find(font);
    if (it == _fonts.end()) return;
    UnloadFont(it->second);
    _fonts.erase(it);
}

Vector2f RaylibRenderer::worldToScreen(Vector3f worldPos) const
{
    ::Vector3 v = { worldPos.x, worldPos.y, worldPos.z };
    ::Vector2 s = GetWorldToScreen(v, _currentCamera);
    return { s.x, s.y };
}

Vector3f RaylibRenderer::screenToWorldRay(Vector2f screenPos) const
{
    ::Ray ray = GetMouseRay({screenPos.x, screenPos.y}, _currentCamera);
    return { ray.direction.x, ray.direction.y, ray.direction.z };
}

RayCollisionData RaylibRenderer::checkRayMeshCollision(
    Vector3f rayOrigin, Vector3f rayDirection,
    MeshHandle mesh, Vector3f meshPosition)
{
    RayCollisionData result{};
    auto it = _models.find(mesh);
    if (it == _models.end()) return result;

    ::Ray ray = { {rayOrigin.x, rayOrigin.y, rayOrigin.z},
                  {rayDirection.x, rayDirection.y, rayDirection.z} };
    ::Matrix transform = MatrixTranslate(meshPosition.x, meshPosition.y, meshPosition.z);
    ::RayCollision col = GetRayCollisionMesh(ray, it->second.meshes[0], transform);

    result.hasHit      = col.hit;
    result.distance    = col.distance;
    result.hitPosition = { col.point.x,  col.point.y,  col.point.z };
    result.hitUV       = { col.normal.x, col.normal.y };
    return result;
}

} // namespace graphic::raylib
