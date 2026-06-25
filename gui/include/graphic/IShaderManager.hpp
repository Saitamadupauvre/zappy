#pragma once
#include "graphic/Types.hpp"
#include <string>

namespace graphic {

class IShaderManager {
public:
    virtual ~IShaderManager() = default;

    virtual ShaderHandle load(const std::string& name, const std::string& vsPath, const std::string& fsPath) = 0;
    virtual void unload(const std::string& name) = 0;
};

} // namespace graphic