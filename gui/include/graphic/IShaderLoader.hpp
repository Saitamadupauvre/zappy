#pragma once
#include <string>
#include "graphic/Types.hpp"

namespace graphic {

class IShaderLoader {
public:
    virtual ~IShaderLoader() = default;

    virtual ShaderHandle load(const std::string& vsPath, const std::string& fsPath) = 0;
};

} // namespace graphic 