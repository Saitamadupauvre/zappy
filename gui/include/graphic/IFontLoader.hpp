#pragma once

#include "graphic/Types.hpp"
#include <string>

namespace graphic {

class IFontLoader {
public:
    virtual ~IFontLoader() = default;

    virtual FontData loadFromFile  (const std::string& path, int fontSize, bool cjk = false) = 0;
    virtual FontData loadFromMemory(const uint8_t* data, size_t size, int fontSize, bool cjk = false) = 0;
};

} // namespace graphic
