#include "RaylibTextureLoader.hpp"
#include <raylib.h>
#include <cstring>

namespace graphic::raylib {

TextureData RaylibTextureLoader::loadFromFile(const std::string& path)
{
    ::Image img = LoadImage(path.c_str());
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    TextureData out;
    out.width    = img.width;
    out.height   = img.height;
    out.channels = 4;
    size_t bytes = static_cast<size_t>(img.width * img.height * 4);
    out.pixels.resize(bytes);
    std::memcpy(out.pixels.data(), img.data, bytes);

    UnloadImage(img);
    return out;
}

TextureData RaylibTextureLoader::loadFromMemory(const uint8_t* data, size_t size)
{
    ::Image img = LoadImageFromMemory(".png", data, static_cast<int>(size));
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    TextureData out;
    out.width    = img.width;
    out.height   = img.height;
    out.channels = 4;
    size_t bytes = static_cast<size_t>(img.width * img.height * 4);
    out.pixels.resize(bytes);
    std::memcpy(out.pixels.data(), img.data, bytes);

    UnloadImage(img);
    return out;
}

} // namespace graphic::raylib
