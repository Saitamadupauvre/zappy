#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <optional>
#include <variant>
#include "graphic/Math.hpp"

namespace graphic {

struct Color4b {
    unsigned char r, g, b, a;
    static constexpr Color4b white()       { return {255, 255, 255, 255}; }
    static constexpr Color4b black()       { return {  0,   0,   0, 255}; }
    static constexpr Color4b transparent() { return {  0,   0,   0,   0}; }
    static constexpr Color4b red()         { return {255,   0,   0, 255}; }
    static constexpr Color4b green()       { return {  0, 255,   0, 255}; }
    static constexpr Color4b blue()        { return {  0,   0, 255, 255}; }
};

struct BoundingBox3D { Vector3f min; Vector3f max; };
struct Rectangle2D   { Vector2f position; Vector2f size; };

struct MeshHandle    { uint32_t id = 0; bool operator==(const MeshHandle&)    const = default; };
struct TextureHandle { uint32_t id = 0; bool operator==(const TextureHandle&) const = default; };
struct FontHandle    { uint32_t id = 0; bool operator==(const FontHandle&)    const = default; };

struct TextureData {
    std::vector<uint8_t> pixels;
    int width    = 0;
    int height   = 0;
    int channels = 4; // RGBA
};

struct GlyphRect {
    int      codepoint;
    float    u0, v0, u1, v1;
    float    advanceX;
    float    offsetX, offsetY;
};

struct FontData {
    TextureData            atlas;
    std::vector<GlyphRect> glyphs;
    int                    size = 16;
    float                  lineHeight = 0.f;
};

struct CameraState {
    Vector3f position  = {0.f, 10.f, 10.f};
    Vector3f target    = {0.f,  0.f,  0.f};
    Vector3f up        = {0.f,  1.f,  0.f};
    float    fovDegrees = 45.f;
};

struct MeshDrawParams {
    MeshHandle    mesh;
    TextureHandle texture;
    Matrix4x4     transform = Matrix4x4::Identity();
    Color4b       tint      = Color4b::white();
    bool          wireframe = false;

    static MeshDrawParams simple(
        MeshHandle mesh, TextureHandle texture,
        Vector3f pos, Vector3f scale = Vector3f::one(),
        float rotAngleY = 0.f,
        Color4b tint = Color4b::white())
    {
        Matrix4x4 t = Matrix4x4::Translation(pos.x, pos.y, pos.z);
        Matrix4x4 r = Matrix4x4::RotationY(rotAngleY);
        Matrix4x4 s = Matrix4x4::Scale(scale.x, scale.y, scale.z);
        return { mesh, texture, t * r * s, tint };
    }
};

struct SolidFill {
    Color4b color = Color4b::white();
};
struct TextureFill {
    TextureHandle texture;
    Rectangle2D   uvCrop = {{0.f,0.f},{1.f,1.f}};
    Color4b       tint   = Color4b::white();
};
using Fill = std::variant<SolidFill, TextureFill>;

struct Stroke {
    Fill  paint = SolidFill{Color4b::black()};
    float width = 1.f;
};

struct ShadowEffect {
    Color4b  color  = {0, 0, 0, 120};
    Vector2f offset = {2.f, 2.f};
    float    blur   = 4.f;
};
using Effect = std::variant<ShadowEffect>;

struct ShapeStyle {
    std::optional<Fill>   fill;
    std::optional<Stroke> stroke;
    std::vector<Effect>   effects;
    float cornerRadius = 0.f;
    float opacity      = 1.f;
};

struct TextStyle {
    FontHandle              font;
    int                     size    = 16;
    Color4b                 color   = Color4b::white();
    std::optional<Stroke>   outline;
    std::optional<ShadowEffect> shadow;
    float                   opacity = 1.f;
};

struct SpriteDrawParams {
    TextureHandle texture;
    Rectangle2D   srcRect = {{0.f,0.f},{1.f,1.f}}; // normalized UV
    Rectangle2D   dstRect;
    Color4b       tint    = Color4b::white();
    float         rotation = 0.f;
    float         opacity  = 1.f;
};

enum class KeyCode {
    UNKNOWN = 0,
    KEY_ESCAPE, KEY_SPACE, KEY_ENTER, KEY_BACKSPACE,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
};

enum class MouseBtn { LEFT, RIGHT, MIDDLE };

struct MouseData {
    Vector2f screenPosition;
};

struct VertexData {
    std::vector<Vector3f>       positions;
    std::vector<Vector2f>       texCoords;
    std::vector<Vector3f>       normals;
    std::vector<unsigned short> indices;
};

struct RayCollisionData {
    bool     hasHit      = false;
    float    distance    = 0.f;
    Vector3f hitPosition = {};
    Vector2f hitUV       = {};
};

enum class RendererCap {
    RoundedCorners,
    DropShadow,
    TextOutline,
    Instancing,
    CustomShaders,
};

} // namespace graphic

namespace std {
template<> struct hash<graphic::MeshHandle> {
    size_t operator()(graphic::MeshHandle h) const noexcept { return hash<uint32_t>{}(h.id); }
};
template<> struct hash<graphic::TextureHandle> {
    size_t operator()(graphic::TextureHandle h) const noexcept { return hash<uint32_t>{}(h.id); }
};
template<> struct hash<graphic::FontHandle> {
    size_t operator()(graphic::FontHandle h) const noexcept { return hash<uint32_t>{}(h.id); }
};

} // namespace std
