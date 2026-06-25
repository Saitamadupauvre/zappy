#include "RaylibShaderLoader.hpp"

namespace graphic::raylib {

ShaderHandle RaylibShaderLoader::load(const std::string& vsPath, const std::string& fsPath)
{
    ::Shader shader = ::LoadShader(vsPath.c_str(), fsPath.c_str());
    
    return ShaderHandle{ static_cast<uint32_t>(shader.id) };
}

} // namespace graphic::raylib