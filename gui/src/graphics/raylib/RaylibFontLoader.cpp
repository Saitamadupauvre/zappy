#include "RaylibFontLoader.hpp"
#include <raylib.h>
#include <cstring>
#include <vector>

namespace graphic::raylib {

static std::vector<int> buildCodepoints(bool extended = false)
{
    std::vector<int> cp;
    // Basic Latin + Latin-1 Supplement + Latin Extended-A/B
    for (int i = 0x0020; i <= 0x024F; ++i) cp.push_back(i);
    // Latin Extended Additional (Vietnamese precomposed, etc.)
    for (int i = 0x1E00; i <= 0x1EFF; ++i) cp.push_back(i);
    // General Punctuation, arrows, bullets
    for (int i = 0x2000; i <= 0x206F; ++i) cp.push_back(i);
    if (extended) {
        // Arabic
        for (int i = 0x0600; i <= 0x06FF; ++i) cp.push_back(i);
        // Arabic Presentation Forms-A
        for (int i = 0xFB50; i <= 0xFDFF; ++i) cp.push_back(i);
        // Arabic Presentation Forms-B
        for (int i = 0xFE70; i <= 0xFEFF; ++i) cp.push_back(i);
        // Devanagari (Hindi)
        for (int i = 0x0900; i <= 0x097F; ++i) cp.push_back(i);
        // Cyrillic (Russian)
        for (int i = 0x0400; i <= 0x04FF; ++i) cp.push_back(i);
        // CJK Symbols & Punctuation
        for (int i = 0x3000; i <= 0x303F; ++i) cp.push_back(i);
        // Hiragana
        for (int i = 0x3040; i <= 0x309F; ++i) cp.push_back(i);
        // Katakana
        for (int i = 0x30A0; i <= 0x30FF; ++i) cp.push_back(i);
        // CJK Unified Ideographs (main block)
        for (int i = 0x4E00; i <= 0x9FFF; ++i) cp.push_back(i);
        // Halfwidth & Fullwidth Forms
        for (int i = 0xFF00; i <= 0xFFEF; ++i) cp.push_back(i);
        // Hangul Syllables (Korean)
        for (int i = 0xAC00; i <= 0xD7A3; ++i) cp.push_back(i);
        // Hangul Jamo
        for (int i = 0x1100; i <= 0x11FF; ++i) cp.push_back(i);
    }
    return cp;
}

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

FontData RaylibFontLoader::loadFromFile(const std::string& path, int fontSize, bool cjk)
{
    auto cp = buildCodepoints(cjk);
    return convertRaylibFont(LoadFontEx(path.c_str(), fontSize, cp.data(), static_cast<int>(cp.size())));
}

FontData RaylibFontLoader::loadFromMemory(const uint8_t* data, size_t size, int fontSize, bool cjk)
{
    auto cp = buildCodepoints(cjk);
    return convertRaylibFont(
        LoadFontFromMemory(".ttf", data, static_cast<int>(size), fontSize, cp.data(), static_cast<int>(cp.size()))
    );
}

} // namespace graphic::raylib
