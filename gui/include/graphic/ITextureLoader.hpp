#pragma once

#include "graphic/Types.hpp"
#include <string>
#include <cstddef>

namespace graphic {

class ITextureLoader {
public:
    virtual ~ITextureLoader() = default;

    virtual TextureData loadFromFile  (const std::string& path)                      = 0;
    virtual TextureData loadFromMemory(const uint8_t* data, size_t size)             = 0;
};

} // namespace graphic
