#pragma once
#include "graphic/IShaderManager.hpp"
#include "raylib.h"
#include <unordered_map>
#include <string>

namespace graphic::raylib {

class RaylibShaderManager : public IShaderManager {
public:
    RaylibShaderManager() = default;
    ~RaylibShaderManager() override;

    ShaderHandle load(const std::string& name, const std::string& vsPath, const std::string& fsPath) override;
    void unload(const std::string& name) override;

    ::Shader getRawShader(const std::string& name) const;
    int      getCachedLocation(const std::string& shaderName, const std::string& uniform);

private:
    std::unordered_map<std::string, ::Shader> _shaders;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> _locCache;
};

} // namespace graphic::raylib