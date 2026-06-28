#pragma once
#include "graphic/IFontLoader.hpp"

namespace graphic::raylib {

class RaylibFontLoader : public IFontLoader {
public:
    RaylibFontLoader() = default;
    ~RaylibFontLoader() override = default;

    FontData loadFromFile  (const std::string& path, int fontSize, bool cjk = false)           override;
    FontData loadFromMemory(const uint8_t* data, size_t size, int fontSize, bool cjk = false)  override;
};

} // namespace graphic::raylib
