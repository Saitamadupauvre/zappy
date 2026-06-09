#pragma once
#include "graphic/ITextureLoader.hpp"

namespace graphic::raylib {

class RaylibTextureLoader : public ITextureLoader {
public:
    RaylibTextureLoader() = default;
    ~RaylibTextureLoader() override = default;

    TextureData loadFromFile  (const std::string& path)             override;
    TextureData loadFromMemory(const uint8_t* data, size_t size)    override;
};

} // namespace graphic::raylib
