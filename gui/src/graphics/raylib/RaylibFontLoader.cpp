#include "RaylibFontLoader.hpp"
#include <raylib.h>
#include <cstring>

namespace graphic::raylib {

static FontData convertRaylibFont(::Font font)
{
    FontData out;
    out.size       = font.baseSize;
    out.lineHeight = static_cast<float>(font.baseSize);

    ::Image atlasImg = LoadImageFromTexture(font.texture);
    ImageFormat(&atlasImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    out.atlas.width    = atlasImg.width;
    out.atlas.height   = atlasImg.height;
    out.atlas.channels = 4;
    size_t bytes = static_cast<size_t>(atlasImg.width * atlasImg.height * 4);
    out.atlas.pixels.resize(bytes);
    std::memcpy(out.atlas.pixels.data(), atlasImg.data, bytes);
    UnloadImage(atlasImg);

    float atlasW = static_cast<float>(font.texture.width);
    float atlasH = static_cast<float>(font.texture.height);
    out.glyphs.reserve(static_cast<size_t>(font.glyphCount));

    for (int i = 0; i < font.glyphCount; ++i) {
        const ::GlyphInfo& gi  = font.glyphs[i];
        const ::Rectangle& rec = font.recs[i];
        GlyphRect g;
        g.codepoint = gi.value;
        g.u0        = rec.x / atlasW;
        g.v0        = rec.y / atlasH;
        g.u1        = (rec.x + rec.width)  / atlasW;
        g.v1        = (rec.y + rec.height) / atlasH;
        g.advanceX  = (gi.advanceX != 0) ? static_cast<float>(gi.advanceX) : rec.width + 1.f;
        g.offsetX   = static_cast<float>(gi.offsetX);
        g.offsetY   = static_cast<float>(gi.offsetY);
        out.glyphs.push_back(g);
    }

    UnloadFont(font);
    return out;
}

FontData RaylibFontLoader::loadFromFile(const std::string& path, int fontSize)
{
    return convertRaylibFont(LoadFontEx(path.c_str(), fontSize, nullptr, 0));
}

FontData RaylibFontLoader::loadFromMemory(const uint8_t* data, size_t size, int fontSize)
{
    return convertRaylibFont(
        LoadFontFromMemory(".ttf", data, static_cast<int>(size), fontSize, nullptr, 0)
    );
}

} // namespace graphic::raylib
