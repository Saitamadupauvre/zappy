#include "RaylibShaderManager.hpp"

namespace graphic::raylib {

RaylibShaderManager::~RaylibShaderManager() {
    for (auto& [name, shader] : _shaders) {
        ::UnloadShader(shader);
    }
}

ShaderHandle RaylibShaderManager::load(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    if (_shaders.find(name) != _shaders.end()) {
        return { static_cast<uint32_t>(_shaders[name].id) };
    }

    ::Shader shader = ::LoadShader(vsPath.c_str(), fsPath.c_str());
    _shaders[name] = shader;
    
    return { static_cast<uint32_t>(shader.id) };
}

void RaylibShaderManager::unload(const std::string& name) {
    auto it = _shaders.find(name);
    if (it != _shaders.end()) {
        ::UnloadShader(it->second);
        _shaders.erase(it);
    }
}

::Shader RaylibShaderManager::getRawShader(const std::string& name) const {
    auto it = _shaders.find(name);
    if (it != _shaders.end()) {
        return it->second;
    }
    return {};
}

int RaylibShaderManager::getCachedLocation(const std::string& shaderName, const std::string& uniform)
{
    auto& uniformMap = _locCache[shaderName];
    auto  uit        = uniformMap.find(uniform);
    if (uit != uniformMap.end())
        return uit->second;

    auto sit = _shaders.find(shaderName);
    if (sit == _shaders.end()) return -1;

    int loc = ::GetShaderLocation(sit->second, uniform.c_str());
    uniformMap[uniform] = loc;
    return loc;
}

} // namespace graphic::raylib