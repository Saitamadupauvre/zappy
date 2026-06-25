#pragma once

#include "graphic/IShaderLoader.hpp"
#include "raylib.h"

namespace graphic::raylib {

class RaylibShaderLoader : public IShaderLoader {
public:
    RaylibShaderLoader() = default;
    ~RaylibShaderLoader() override = default;

    ShaderHandle load(const std::string& vsPath, const std::string& fsPath) override;
};

} // namespace graphic::raylib  