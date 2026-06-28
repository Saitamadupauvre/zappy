#include "RaylibRenderer.hpp"
#include "util/Overloaded.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <iostream>
#include <vector>
#include <algorithm>


namespace graphic::raylib {

RaylibRenderer::RaylibRenderer()
{
    if (IsWindowReady()) {
        init();
    }
}

void RaylibRenderer::init() {
    _shaderManager.load("outline", "assets/shaders/outline.vs", "assets/shaders/outline.fs");
    _shaderManager.load("grass",   "assets/shaders/grass.vs",   "assets/shaders/grass.fs");
    _shaderManager.load("crystal", "assets/shaders/crystal.vs", "assets/shaders/crystal.fs");
    _shaderManager.load("skybox",  "assets/shaders/skybox.vs",  "assets/shaders/skybox.fs");
    _shaderManager.load("earth",       "assets/shaders/earth.vs",       "assets/shaders/earth.fs");
    _shaderManager.load("minimal_sky", "assets/shaders/minimal_sky.vs", "assets/shaders/minimal_sky.fs");
    _shaderManager.load("city",        "assets/shaders/city.vs",        "assets/shaders/city.fs");
    _shaderManager.load("wave",    "assets/shaders/wave.vs",    "assets/shaders/wave.fs");
    _shaderManager.load("ritual",    "assets/shaders/ritual.vs",    "assets/shaders/ritual.fs");
    _shaderManager.load("explosion", "assets/shaders/explosion.vs", "assets/shaders/explosion.fs");
}


RaylibRenderer::~RaylibRenderer()
{
    for (auto& [id, model]   : _models)
        if (!_meshViews.count(id)) UnloadModel(model);
    for (auto& [id, entry]   : _loadedModels) {
        if (entry.anims) UnloadModelAnimations(entry.anims, entry.animCount);
        UnloadModel(entry.model);
    }
    for (auto& [id, texture] : _textures) UnloadTexture(texture);
    for (auto& [id, font]    : _fonts)    UnloadFont(font);
    for (auto& [id, grass]   : _grassFields) {
        if (grass.colorVboId) rlUnloadVertexBuffer(grass.colorVboId);
        UnloadMesh(grass.mesh);
        // Material shader is owned by _shaderManager; unload the rest.
        grass.material.shader = {};
        UnloadMaterial(grass.material);
    }
    for (auto& [id, sky] : _skyboxes) {
        sky.model.materials[0].shader = {}; // shader owned by _shaderManager
        UnloadModel(sky.model);
    }
}

bool RaylibRenderer::supports(RendererCap cap) const
{
    switch (cap) {
        case RendererCap::RoundedCorners: return true;
        case RendererCap::DropShadow:     return true;
        case RendererCap::TextOutline:    return true;
        case RendererCap::Instancing:     return true;
        case RendererCap::CustomShaders:  return true;
    }
    return false;
}

void RaylibRenderer::clear(Color4b color)
{
    ClearBackground(toRayColor(color));
}

Vector2f RaylibRenderer::getViewportSize() const
{
    return { static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) };
}

void RaylibRenderer::setCamera(const CameraState& cam)
{
    _cameraState = cam;
    _currentCamera.position   = { cam.position.x, cam.position.y, cam.position.z };
    _currentCamera.target     = { cam.target.x, cam.target.y, cam.target.z };
    _currentCamera.up         = { cam.up.x, cam.up.y, cam.up.z };
    _currentCamera.fovy       = cam.fovDegrees;
    _currentCamera.projection = CAMERA_PERSPECTIVE;
}

CameraState RaylibRenderer::getCamera() const
{
    return _cameraState;
}

void RaylibRenderer::begin3D()
{
    _in3D = true;
    BeginMode3D(_currentCamera);
}

void RaylibRenderer::end3D()
{
    _in3D = false;
    EndMode3D();
}

void RaylibRenderer::drawOutline(const MeshDrawParams& p, [[maybe_unused]] float thickness, [[maybe_unused]] Color4b color)
{
    auto it = _models.find(p.mesh);
    if (it == _models.end()) return;
    ::Model model = it->second;

    const float* m = p.transform.m;
    model.transform = ::Matrix{m[0], m[4], m[8], m[12], m[1], m[5], m[9], m[13], m[2], m[6], m[10], m[14], m[3], m[7], m[11], m[15]};

    ::Shader outlineShader = _shaderManager.getRawShader("outline");
    ::BeginShaderMode(outlineShader);

    float colVec[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
    int colorLoc = _shaderManager.getCachedLocation("outline", "outlineColor");
    int thickLoc = _shaderManager.getCachedLocation("outline", "thickness");
    ::SetShaderValue(outlineShader, colorLoc, colVec, SHADER_UNIFORM_VEC4);
    ::SetShaderValue(outlineShader, thickLoc, &thickness, SHADER_UNIFORM_FLOAT);

    // Swap every material to the outline shader so all meshes of a multi-material
    // model get inflated, not just the first one (otherwise the rest render solid
    // on top of the model and it looks recolored instead of outlined).
    _shaderDefaultsBuf.resize(model.materialCount);
    for (int i = 0; i < model.materialCount; ++i) {
        _shaderDefaultsBuf[i] = model.materials[i].shader;
        model.materials[i].shader = outlineShader;
    }

    // Inverted-hull outline: keep only the faces pointing away from the camera.
    // Screen-space winding is flipped by a left-handed (negative-determinant)
    // transform, so the cull face follows the transform handedness — this works
    // for every mesh/model regardless of which orientation path built the matrix.
    float det = m[0] * (m[5] * m[10] - m[6] * m[9])
              - m[1] * (m[4] * m[10] - m[6] * m[8])
              + m[2] * (m[4] * m[9]  - m[5] * m[8]);

    ::rlEnableBackfaceCulling();
    ::rlSetCullFace(det >= 0.0f ? RL_CULL_FACE_FRONT : RL_CULL_FACE_BACK);

    ::DrawModel(model, {0.f, 0.f, 0.f}, 1.0f, WHITE);

    ::rlSetCullFace(RL_CULL_FACE_BACK);
    ::rlDisableBackfaceCulling();

    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = _shaderDefaultsBuf[i];
    ::EndShaderMode();
}

namespace {

// Build and GPU-upload a raylib Mesh from engine VertexData (shared shape of
// RaylibRenderer::uploadMesh, but returns the raw Mesh for instanced use).
::Mesh buildRayMesh(const VertexData& data)
{
    ::Mesh m{};
    m.vertexCount   = static_cast<int>(data.positions.size());
    m.triangleCount = static_cast<int>(data.indices.size() / 3);

    m.vertices = static_cast<float*>(MemAlloc(m.vertexCount * 3 * sizeof(float)));
    for (int i = 0; i < m.vertexCount; ++i) {
        m.vertices[i*3]   = data.positions[i].x;
        m.vertices[i*3+1] = data.positions[i].y;
        m.vertices[i*3+2] = data.positions[i].z;
    }
    if (!data.texCoords.empty()) {
        m.texcoords = static_cast<float*>(MemAlloc(m.vertexCount * 2 * sizeof(float)));
        for (int i = 0; i < m.vertexCount; ++i) {
            m.texcoords[i*2]   = data.texCoords[i].x;
            m.texcoords[i*2+1] = data.texCoords[i].y;
        }
    }
    if (!data.normals.empty()) {
        m.normals = static_cast<float*>(MemAlloc(m.vertexCount * 3 * sizeof(float)));
        for (int i = 0; i < m.vertexCount; ++i) {
            m.normals[i*3]   = data.normals[i].x;
            m.normals[i*3+1] = data.normals[i].y;
            m.normals[i*3+2] = data.normals[i].z;
        }
    }
    m.indices = static_cast<unsigned short*>(MemAlloc(m.triangleCount * 3 * sizeof(unsigned short)));
    for (int i = 0; i < m.triangleCount * 3; ++i)
        m.indices[i] = data.indices[i];

    UploadMesh(&m, false);
    return m;
}

// Engine row-major (row-vector) matrix → raylib Matrix, same mapping drawMesh
// uses. raylib's Matrix follows raymath convention (translation in fields
// m12/m13/m14), so the row→column swap below is what places translation where
// the GPU expects it. Pair this with a scale*rot*trans compose order — a
// trans*rot*scale order instead bakes the translation through the scale and
// collapses every blade onto the origin.
::Matrix toRayMatrix(const Matrix4x4& t)
{
    const float* m = t.m;
    return ::Matrix{
        m[0],  m[4],  m[8],  m[12],
        m[1],  m[5],  m[9],  m[13],
        m[2],  m[6],  m[10], m[14],
        m[3],  m[7],  m[11], m[15]
    };
}

constexpr int MAX_INSTANCES_PER_BATCH = 65536;

} // namespace

GrassFieldHandle RaylibRenderer::uploadGrassField(const VertexData& bladeMesh,
    const std::vector<Matrix4x4>& transforms,
    const std::vector<Color4b>& colors)
{
    GrassFieldEntry entry;
    entry.mesh     = buildRayMesh(bladeMesh);
    entry.material = LoadMaterialDefault();

    ::Shader grass = _shaderManager.getRawShader("grass");
    entry.material.shader = grass;
    // Tell raylib which vertex attribute receives the per-instance model matrix.
    entry.material.shader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocationAttrib(grass, "instanceTransform");
    entry.instanceColorLoc = GetShaderLocationAttrib(grass, "instanceColor");

    entry.transforms.reserve(transforms.size());
    for (const auto& t : transforms)
        entry.transforms.push_back(toRayMatrix(t));

    // Per-instance vec4 tint buffer, bound to the mesh VAO with attribute
    // divisor = 1 so each blade reads one colour.
    std::vector<float> rgba;
    rgba.reserve(colors.size() * 4);
    for (const auto& c : colors) {
        rgba.push_back(c.r / 255.f);
        rgba.push_back(c.g / 255.f);
        rgba.push_back(c.b / 255.f);
        rgba.push_back(c.a / 255.f);
    }
    if (entry.instanceColorLoc >= 0 && !rgba.empty()) {
        rlEnableVertexArray(entry.mesh.vaoId);
        entry.colorVboId = rlLoadVertexBuffer(rgba.data(),
            static_cast<int>(rgba.size() * sizeof(float)), false);
        rlSetVertexAttribute(entry.instanceColorLoc, 4, RL_FLOAT, false, 0, 0);
        rlSetVertexAttributeDivisor(entry.instanceColorLoc, 1);
        rlEnableVertexAttribute(entry.instanceColorLoc);
        rlDisableVertexArray();
    }

    GrassFieldHandle id = _grassIdCounter;
    _grassIdCounter.id++;
    _grassFields[id] = std::move(entry);
    return id;
}

void RaylibRenderer::drawGrassField(GrassFieldHandle field, const GrassDrawParams& params)
{
    auto it = _grassFields.find(field);
    if (it == _grassFields.end()) return;
    GrassFieldEntry& g = it->second;

    ::Shader shader = g.material.shader;
    int timeLoc = _shaderManager.getCachedLocation("grass", "time");
    int dirLoc  = _shaderManager.getCachedLocation("grass", "windDir");
    int strLoc  = _shaderManager.getCachedLocation("grass", "windStrength");
    float t   = params.time;
    float dir[2] = { params.windDir.x, params.windDir.y };
    float str = params.windStrength;
    SetShaderValue(shader, timeLoc, &t,   SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, dirLoc,  dir,  SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, strLoc,  &str, SHADER_UNIFORM_FLOAT);

    const int total = static_cast<int>(g.transforms.size());
    for (int start = 0; start < total; start += MAX_INSTANCES_PER_BATCH) {
        int count = std::min(MAX_INSTANCES_PER_BATCH, total - start);
        // Re-point the per-instance colour attribute at this chunk's slice so the
        // tint stays aligned with the transform slice we draw.
        if (g.colorVboId && g.instanceColorLoc >= 0) {
            rlEnableVertexArray(g.mesh.vaoId);
            rlEnableVertexBuffer(g.colorVboId);
            rlSetVertexAttribute(g.instanceColorLoc, 4, RL_FLOAT, false, 0,
                                 start * 4 * static_cast<int>(sizeof(float)));
            rlSetVertexAttributeDivisor(g.instanceColorLoc, 1);
            rlDisableVertexArray();
        }
        DrawMeshInstanced(g.mesh, g.material, &g.transforms[start], count);
    }
}

void RaylibRenderer::unloadGrassField(GrassFieldHandle field)
{
    auto it = _grassFields.find(field);
    if (it == _grassFields.end()) return;
    if (it->second.colorVboId) rlUnloadVertexBuffer(it->second.colorVboId);
    UnloadMesh(it->second.mesh);
    it->second.material.shader = {}; // shader owned by _shaderManager
    UnloadMaterial(it->second.material);
    _grassFields.erase(it);
}

SkyboxHandle RaylibRenderer::uploadSkybox()
{
    SkyboxEntry entry;
    entry.model = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    entry.model.materials[0].shader = _shaderManager.getRawShader("skybox");
    SkyboxHandle handle = _skyboxIdCounter;
    _skyboxIdCounter.id++;
    _skyboxes[handle] = std::move(entry);
    return handle;
}

void RaylibRenderer::drawSkybox(SkyboxHandle handle, float time)
{
    auto it = _skyboxes.find(handle);
    if (it == _skyboxes.end()) return;
    auto& entry = it->second;
    const std::string& sname = entry.currentShaderName;
    int timeLoc = _shaderManager.getCachedLocation(sname, "time");
    SetShaderValue(entry.model.materials[0].shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
    if (sname == "city") {
        auto vp = getViewportSize();
        float res[2] = { vp.x, vp.y };
        int resLoc = _shaderManager.getCachedLocation("city", "iResolution");
        SetShaderValue(entry.model.materials[0].shader, resLoc, res, SHADER_UNIFORM_VEC2);
    }
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    DrawModel(entry.model,
              {_currentCamera.position.x, _currentCamera.position.y, _currentCamera.position.z},
              1.f, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

void RaylibRenderer::unloadSkybox(SkyboxHandle handle)
{
    auto it = _skyboxes.find(handle);
    if (it == _skyboxes.end()) return;
    it->second.model.materials[0].shader = {}; // shader owned by _shaderManager
    UnloadModel(it->second.model);
    _skyboxes.erase(it);
}

void RaylibRenderer::setSkyboxShader(SkyboxHandle handle, const std::string& shaderName)
{
    auto it = _skyboxes.find(handle);
    if (it == _skyboxes.end()) return;
    it->second.model.materials[0].shader = _shaderManager.getRawShader(shaderName);
    it->second.currentShaderName = shaderName;
}

void RaylibRenderer::drawWave(const WaveDrawParams& p)
{
    auto it = _models.find(p.groundMesh);
    if (it == _models.end()) return;

    ::Shader shader = _shaderManager.getRawShader("wave");

    float elapsed   = p.elapsed;
    float duration  = p.duration;
    float maxRadius = p.maxRadius;
    float center[3] = { p.center.x, p.center.y, p.center.z };
    float color[4]  = { p.color.r / 255.f, p.color.g / 255.f,
                        p.color.b / 255.f, p.color.a / 255.f };

    SetShaderValue(shader, _shaderManager.getCachedLocation("wave", "uCenter"),    center,    SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, _shaderManager.getCachedLocation("wave", "uElapsed"),   &elapsed,  SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, _shaderManager.getCachedLocation("wave", "uDuration"),  &duration, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, _shaderManager.getCachedLocation("wave", "uMaxRadius"), &maxRadius,SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, _shaderManager.getCachedLocation("wave", "uColor"),      color,    SHADER_UNIFORM_VEC4);

    // Swap ground mesh material to the wave shader, draw it, restore.
    ::Model model = it->second;
    model.transform = MatrixIdentity();

    _shaderDefaultsBuf.resize(model.materialCount);
    for (int i = 0; i < model.materialCount; ++i) {
        _shaderDefaultsBuf[i] = model.materials[i].shader;
        model.materials[i].shader = shader;
    }

    rlDisableDepthMask();
    rlSetBlendMode(BLEND_ALPHA);
    BeginBlendMode(BLEND_ALPHA);
    DrawModel(model, {0.f, 0.f, 0.f}, 1.f, WHITE);
    EndBlendMode();
    rlEnableDepthMask();

    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = _shaderDefaultsBuf[i];
}

void RaylibRenderer::drawRitual(const RitualDrawParams& p)
{
    auto it = _models.find(p.groundMesh);
    if (it == _models.end()) return;

    ::Shader shader = _shaderManager.getRawShader("ritual");
    if (shader.id == 0) return;

    float time   = p.time;
    float radius = p.radius;
    float center[3] = { p.center.x, p.center.y, p.center.z };
    float normal[3] = { p.surfaceNormal.x, p.surfaceNormal.y, p.surfaceNormal.z };
    float color[4]  = { p.color.r / 255.f, p.color.g / 255.f,
                        p.color.b / 255.f, p.color.a / 255.f };

    SetShaderValue(shader, _shaderManager.getCachedLocation("ritual", "uCenter"),        center, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, _shaderManager.getCachedLocation("ritual", "uSurfaceNormal"), normal, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, _shaderManager.getCachedLocation("ritual", "uTime"),   &time,  SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, _shaderManager.getCachedLocation("ritual", "uRadius"), &radius,SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, _shaderManager.getCachedLocation("ritual", "uColor"),   color, SHADER_UNIFORM_VEC4);

    ::Model model = it->second;
    model.transform = MatrixIdentity();

    _shaderDefaultsBuf.resize(model.materialCount);
    for (int i = 0; i < model.materialCount; ++i) {
        _shaderDefaultsBuf[i] = model.materials[i].shader;
        model.materials[i].shader = shader;
    }

    rlDisableDepthMask();
    rlSetBlendMode(BLEND_ALPHA);
    BeginBlendMode(BLEND_ALPHA);
    DrawModel(model, {0.f, 0.f, 0.f}, 1.f, WHITE);
    EndBlendMode();
    rlEnableDepthMask();

    for (int i = 0; i < model.materialCount; ++i)
        model.materials[i].shader = _shaderDefaultsBuf[i];
}

void RaylibRenderer::drawLine3D(Vector3f from, Vector3f to, Color4b color)
{
    rlDisableDepthMask();
    BeginBlendMode(BLEND_ADDITIVE);
    ::DrawLine3D({from.x, from.y, from.z}, {to.x, to.y, to.z}, toRayColor(color));
    EndBlendMode();
    rlEnableDepthMask();
}

void RaylibRenderer::drawSphere3D(Vector3f center, float radius, Color4b color)
{
    rlDisableDepthMask();
    BeginBlendMode(BLEND_ADDITIVE);
    ::DrawSphere({center.x, center.y, center.z}, radius, toRayColor(color));
    EndBlendMode();
    rlEnableDepthMask();
}

void RaylibRenderer::drawMesh(const MeshDrawParams& p)
{
    auto it = _models.find(p.mesh);
    if (it == _models.end()) return;

    ::Model model = it->second;
    if (p.texture.id != 0) {
        auto texIt = _textures.find(p.texture);
        if (texIt != _textures.end())
            model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texIt->second;
    }

    const float* m = p.transform.m;
    model.transform = ::Matrix{
        m[0],  m[4],  m[8],  m[12],
        m[1],  m[5],  m[9],  m[13],
        m[2],  m[6],  m[10], m[14],
        m[3],  m[7],  m[11], m[15]
    };

    ::Vector3 origin = {0.f, 0.f, 0.f};
    ::Color   col    = toRayColor(p.tint);

    if (p.wireframe)
        DrawModelWires(model, origin, 1.f, col);
    else
        DrawModel(model, origin, 1.f, col);
}

void RaylibRenderer::drawModel(const ModelDrawParams& p)
{
    auto it = _loadedModels.find(p.model);
    if (it == _loadedModels.end()) return;

    ::Model model = it->second.model;

    const float* m = p.transform.m;
    // Our Matrix4x4 is row-major; transpose to Raylib's column-major layout.
    model.transform = ::Matrix{
        m[0],  m[4],  m[8],  m[12],
        m[1],  m[5],  m[9],  m[13],
        m[2],  m[6],  m[10], m[14],
        m[3],  m[7],  m[11], m[15]
    };

    ::Vector3 origin = {0.f, 0.f, 0.f};

    rlDisableBackfaceCulling();
    if (!p.meshTints.empty() || !p.meshShaders.empty()) {
        // Per-mesh rendering: override diffuse color and optionally swap shader +
        // enable alpha-blend per mesh. Restores all state after each mesh draw.
        for (int i = 0; i < model.meshCount; ++i) {
            int matIdx = model.meshMaterial[i];
            ::Material& mat = model.materials[matIdx];

            // --- tint ---
            // alpha==0 sentinel means "preserve GLB material color, don't override".
            // Out-of-range meshes also preserve material color when meshTints is set.
            static const graphic::Color4b kNoOverride{0, 0, 0, 0};
            const graphic::Color4b& tc = (i < (int)p.meshTints.size()) ? p.meshTints[i]
                                       : (!p.meshTints.empty()         ? kNoOverride
                                                                       : p.tint);
            ::Color savedDiffuse = mat.maps[MATERIAL_MAP_DIFFUSE].color;
            bool overrideDiffuse = (tc.a != 0);
            if (overrideDiffuse)
                mat.maps[MATERIAL_MAP_DIFFUSE].color = toRayColor(tc);

            // --- shader override ---
            ::Shader savedShader = mat.shader;
            bool hasShader = (i < (int)p.meshShaders.size() && !p.meshShaders[i].empty());
            if (hasShader) {
                ::Shader sh = _shaderManager.getRawShader(p.meshShaders[i]);
                mat.shader  = sh;
                // Pass alpha + emissive to the shader
                float a = tc.a / 255.0f;
                float e = 0.45f;
                int aLoc = _shaderManager.getCachedLocation(p.meshShaders[i], "alpha");
                int eLoc = _shaderManager.getCachedLocation(p.meshShaders[i], "emissiveStrength");
                if (aLoc >= 0) SetShaderValue(sh, aLoc, &a, SHADER_UNIFORM_FLOAT);
                if (eLoc >= 0) SetShaderValue(sh, eLoc, &e, SHADER_UNIFORM_FLOAT);
            }

            // --- blend mode ---
            bool transparent = overrideDiffuse && (tc.a < 255);
            if (transparent) BeginBlendMode(BLEND_ALPHA);

            DrawMesh(model.meshes[i], mat, model.transform);

            if (transparent) EndBlendMode();
            if (hasShader)   mat.shader = savedShader;
            mat.maps[MATERIAL_MAP_DIFFUSE].color = savedDiffuse;
        }
    } else {
        ::Color col = toRayColor(p.tint);
        if (p.wireframe)
            DrawModelWires(model, origin, 1.f, col);
        else
            DrawModel(model, origin, 1.f, col);
    }
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
}

ModelHandle RaylibRenderer::loadModel(const std::string& path)
{
    ModelEntry entry;
    entry.model = LoadModel(path.c_str());
    entry.anims = LoadModelAnimations(path.c_str(), &entry.animCount);

    ModelHandle id = _modelIdCounter;
    _modelIdCounter.id++;
    _loadedModels[id] = entry;
    return id;
}

MeshHandle RaylibRenderer::meshFromModel(ModelHandle model)
{
    auto it = _loadedModels.find(model);
    if (it == _loadedModels.end()) return {};

    MeshHandle id = _meshIdCounter;
    _meshIdCounter.id++;
    // Shallow copy: shares the model's GPU meshes. Tracked as a view so its
    // resources are freed by _loadedModels, never twice.
    _models[id] = it->second.model;
    _meshViews.insert(id);
    return id;
}

int RaylibRenderer::modelAnimationCount(ModelHandle model) const
{
    auto it = _loadedModels.find(model);
    return it == _loadedModels.end() ? 0 : it->second.animCount;
}

int RaylibRenderer::modelAnimationFrameCount(ModelHandle model, int animIndex) const
{
    auto it = _loadedModels.find(model);
    if (it == _loadedModels.end()) return 0;
    if (animIndex < 0 || animIndex >= it->second.animCount) return 0;
#ifdef RAYLIB_ANIM_HAS_FRAMECOUNT
    return it->second.anims[animIndex].frameCount;
#else
    return it->second.anims[animIndex].keyframeCount;
#endif
}

void RaylibRenderer::updateModelAnimation(ModelHandle model, int animIndex, float frame)
{
    auto it = _loadedModels.find(model);
    if (it == _loadedModels.end()) return;
    if (animIndex < 0 || animIndex >= it->second.animCount) return;
    UpdateModelAnimation(it->second.model, it->second.anims[animIndex],
                         static_cast<int>(frame));
}

void RaylibRenderer::unloadModel(ModelHandle model)
{
    auto it = _loadedModels.find(model);
    if (it == _loadedModels.end()) return;
    if (it->second.anims) UnloadModelAnimations(it->second.anims, it->second.animCount);
    UnloadModel(it->second.model);
    _loadedModels.erase(it);
}

void RaylibRenderer::drawGrid(int slices, float spacing)
{
    DrawGrid(slices, spacing);
}

void RaylibRenderer::begin2D() { /* Raylib 2D draws anywhere inside BeginDrawing() */ }
void RaylibRenderer::end2D()   { }
void RaylibRenderer::beginScissor(int x, int y, int w, int h) { BeginScissorMode(x, y, w, h); }
void RaylibRenderer::endScissor() { EndScissorMode(); }

::Rectangle RaylibRenderer::toRayRect(Rectangle2D r, bool /*pixelSpace*/) const
{
    return { r.position.x, r.position.y, r.size.x, r.size.y };
}

static ::Color blendAlpha(::Color c, float opacity)
{
    c.a = static_cast<unsigned char>(c.a * opacity);
    return c;
}

static ::Color resolveFillColor(const Fill& fill, float opacity)
{
    return std::visit(overloaded{
        [&](const SolidFill& f)   -> ::Color { return blendAlpha({f.color.r, f.color.g, f.color.b, f.color.a}, opacity); },
        [&](const TextureFill& f) -> ::Color { return blendAlpha({f.tint.r,  f.tint.g,  f.tint.b,  f.tint.a},  opacity); },
    }, fill);
}

void RaylibRenderer::applyEffects(Rectangle2D rect, const ShapeStyle& style) const
{
    for (const auto& fx : style.effects) {
        std::visit(overloaded{
            [&](const ShadowEffect& s) {
                Rectangle2D shadowRect = {
                    { rect.position.x + s.offset.x, rect.position.y + s.offset.y },
                    rect.size
                };
                ::Color sc = {s.color.r, s.color.g, s.color.b, s.color.a};
                if (style.cornerRadius > 0.f)
                    DrawRectangleRounded(toRayRect(shadowRect), style.cornerRadius / (rect.size.x * 0.5f), 8, sc);
                else
                    DrawRectangleRec(toRayRect(shadowRect), sc);
            }
        }, fx);
    }
}

void RaylibRenderer::applyFillDraw(Rectangle2D rect, const ShapeStyle& style) const
{
    if (!style.fill) return;
    ::Color col = resolveFillColor(*style.fill, style.opacity);

    if (std::holds_alternative<TextureFill>(*style.fill)) {
        const auto& tf = std::get<TextureFill>(*style.fill);
        auto texIt = _textures.find(tf.texture);
        if (texIt != _textures.end()) {
            ::Texture2D tex = texIt->second;
            ::Rectangle src = {
                tf.uvCrop.position.x * tex.width,
                tf.uvCrop.position.y * tex.height,
                tf.uvCrop.size.x     * tex.width,
                tf.uvCrop.size.y     * tex.height
            };
            ::Rectangle dst = toRayRect(rect);
            ::Vector2 origin = {0, 0};
            DrawTexturePro(tex, src, dst, origin, 0.f, col);
            return;
        }
    }

    if (style.cornerRadius > 0.f) {
        float roundness = style.cornerRadius / (std::min(rect.size.x, rect.size.y) * 0.5f);
        DrawRectangleRounded(toRayRect(rect), roundness, 8, col);
    } else {
        DrawRectangleRec(toRayRect(rect), col);
    }
}

void RaylibRenderer::applyStrokeDraw(Rectangle2D rect, const ShapeStyle& style) const
{
    if (!style.stroke) return;
    const auto& stroke = *style.stroke;
    ::Color col = resolveFillColor(stroke.paint, style.opacity);
    if (style.cornerRadius > 0.f) {
        float roundness = style.cornerRadius / (std::min(rect.size.x, rect.size.y) * 0.5f);
        DrawRectangleRoundedLines(toRayRect(rect), roundness, 8, col);
    } else {
        DrawRectangleLinesEx(toRayRect(rect), stroke.width, col);
    }
}

void RaylibRenderer::drawRect(Rectangle2D rect, const ShapeStyle& style)
{
    applyEffects   (rect, style);
    applyFillDraw  (rect, style);
    applyStrokeDraw(rect, style);
}

void RaylibRenderer::drawCircle(Vector2f center, float radius, const ShapeStyle& style)
{
    for (const auto& fx : style.effects) {
        std::visit(overloaded{
            [&](const ShadowEffect& s) {
                ::Color sc = {s.color.r, s.color.g, s.color.b, s.color.a};
                DrawCircleV({center.x + s.offset.x, center.y + s.offset.y}, radius, sc);
            }
        }, fx);
    }

    if (style.fill) {
        ::Color col = resolveFillColor(*style.fill, style.opacity);
        DrawCircleV({center.x, center.y}, radius, col);
    }
    if (style.stroke) {
        ::Color col = resolveFillColor(style.stroke->paint, style.opacity);
        DrawCircleLinesV({center.x, center.y}, radius, col);
    }
}

void RaylibRenderer::drawLine(Vector2f from, Vector2f to, float width, Color4b color)
{
    DrawLineEx({from.x, from.y}, {to.x, to.y}, width, toRayColor(color));
}

void RaylibRenderer::drawSprite(const SpriteDrawParams& p)
{
    auto texIt = _textures.find(p.texture);
    if (texIt == _textures.end()) return;

    ::Texture2D tex = texIt->second;
    ::Rectangle src = {
        p.srcRect.position.x * tex.width,
        p.srcRect.position.y * tex.height,
        p.srcRect.size.x     * tex.width,
        p.srcRect.size.y     * tex.height
    };
    ::Rectangle dst = toRayRect(p.dstRect);
    ::Vector2   origin = {0, 0};
    ::Color     tint   = blendAlpha(toRayColor(p.tint), p.opacity);

    DrawTexturePro(tex, src, dst, origin, p.rotation, tint);
}

void RaylibRenderer::drawText(const std::string& text, Vector2f pos, const TextStyle& style)
{
    ::Font drawFont = GetFontDefault();
    if (style.font.id != 0) {
        auto it = _fonts.find(style.font);
        if (it != _fonts.end()) drawFont = it->second;
    }

    ::Color col = blendAlpha(toRayColor(style.color), style.opacity);

    if (style.shadow) {
        ::Color sc = blendAlpha(toRayColor(style.shadow->color), style.opacity);
        DrawTextEx(drawFont, text.c_str(),
            {pos.x + style.shadow->offset.x, pos.y + style.shadow->offset.y},
            static_cast<float>(style.size), 1.f, sc);
    }

    if (style.outline) {
        ::Color oc = resolveFillColor(style.outline->paint, style.opacity);
        float   ow = style.outline->width;
        for (float dx = -ow; dx <= ow; dx += ow) {
            for (float dy = -ow; dy <= ow; dy += ow) {
                if (dx == 0.f && dy == 0.f) continue;
                DrawTextEx(drawFont, text.c_str(), {pos.x + dx, pos.y + dy},
                    static_cast<float>(style.size), 1.f, oc);
            }
        }
    }

    DrawTextEx(drawFont, text.c_str(), {pos.x, pos.y}, static_cast<float>(style.size), 1.f, col);
}

Vector2f RaylibRenderer::measureText(const std::string& text, const TextStyle& style)
{
    ::Font drawFont = GetFontDefault();
    if (style.font.id != 0) {
        auto it = _fonts.find(style.font);
        if (it != _fonts.end()) drawFont = it->second;
    }
    ::Vector2 sz = MeasureTextEx(drawFont, text.c_str(), static_cast<float>(style.size), 1.f);
    return { sz.x, sz.y };
}

void RaylibRenderer::setDepthTest(bool enabled) {
    if (enabled) {
        rlEnableDepthTest();
    } else {
        rlDisableDepthTest();
    }
}

MeshHandle RaylibRenderer::uploadMesh(const VertexData& data)
{
    ::Mesh rayMesh{};
    rayMesh.vertexCount   = static_cast<int>(data.positions.size());
    rayMesh.triangleCount = static_cast<int>(data.indices.size() / 3);

    rayMesh.vertices = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 3 * sizeof(float)));
    for (int i = 0; i < rayMesh.vertexCount; ++i) {
        rayMesh.vertices[i*3]   = data.positions[i].x;
        rayMesh.vertices[i*3+1] = data.positions[i].y;
        rayMesh.vertices[i*3+2] = data.positions[i].z;
    }
    if (!data.texCoords.empty()) {
        rayMesh.texcoords = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 2 * sizeof(float)));
        for (int i = 0; i < rayMesh.vertexCount; ++i) {
            rayMesh.texcoords[i*2]   = data.texCoords[i].x;
            rayMesh.texcoords[i*2+1] = data.texCoords[i].y;
        }
    }
    if (!data.normals.empty()) {
        rayMesh.normals = static_cast<float*>(MemAlloc(rayMesh.vertexCount * 3 * sizeof(float)));
        for (int i = 0; i < rayMesh.vertexCount; ++i) {
            rayMesh.normals[i*3]   = data.normals[i].x;
            rayMesh.normals[i*3+1] = data.normals[i].y;
            rayMesh.normals[i*3+2] = data.normals[i].z;
        }
    }
    rayMesh.indices = static_cast<unsigned short*>(MemAlloc(rayMesh.triangleCount * 3 * sizeof(unsigned short)));
    for (int i = 0; i < rayMesh.triangleCount * 3; ++i)
        rayMesh.indices[i] = data.indices[i];

    UploadMesh(&rayMesh, false);

    MeshHandle id = _meshIdCounter;
    _meshIdCounter.id++;
    _models[id] = LoadModelFromMesh(rayMesh);
    return id;
}

TextureHandle RaylibRenderer::uploadTexture(const TextureData& data)
{
    ::Image img{};
    img.data    = const_cast<void*>(static_cast<const void*>(data.pixels.data()));
    img.width   = data.width;
    img.height  = data.height;
    img.mipmaps = 1;
    img.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    TextureHandle id = _textureIdCounter;
    _textureIdCounter.id++;
    _textures[id] = LoadTextureFromImage(img);
    return id;
}

FontHandle RaylibRenderer::uploadFont(const FontData& data)
{
    ::Image atlasImg{};
    atlasImg.data    = const_cast<uint8_t*>(data.atlas.pixels.data());
    atlasImg.width   = data.atlas.width;
    atlasImg.height  = data.atlas.height;
    atlasImg.mipmaps = 1;
    atlasImg.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ::Font font{};
    font.baseSize   = data.size;
    font.glyphCount = static_cast<int>(data.glyphs.size());
    font.texture    = LoadTextureFromImage(atlasImg);
    font.recs       = static_cast<::Rectangle*>(MemAlloc(font.glyphCount * sizeof(::Rectangle)));
    font.glyphs     = static_cast<::GlyphInfo*>(MemAlloc(font.glyphCount * sizeof(::GlyphInfo)));

    float atlasW = static_cast<float>(data.atlas.width);
    float atlasH = static_cast<float>(data.atlas.height);
    for (int i = 0; i < font.glyphCount; ++i) {
        const auto& g = data.glyphs[i];
        font.recs[i] = {
            g.u0 * atlasW, g.v0 * atlasH,
            (g.u1 - g.u0) * atlasW, (g.v1 - g.v0) * atlasH
        };
        font.glyphs[i].value    = g.codepoint;
        font.glyphs[i].offsetX  = static_cast<int>(g.offsetX);
        font.glyphs[i].offsetY  = static_cast<int>(g.offsetY);
        font.glyphs[i].advanceX = static_cast<int>(g.advanceX);
        font.glyphs[i].image    = {};
    }

    FontHandle id = _fontIdCounter;
    _fontIdCounter.id++;
    _fonts[id] = font;
    return id;
}

void RaylibRenderer::unloadMesh(MeshHandle mesh)
{
    auto it = _models.find(mesh);
    if (it == _models.end()) return;
    if (!_meshViews.count(mesh)) UnloadModel(it->second);
    _meshViews.erase(mesh);
    _models.erase(it);
}

void RaylibRenderer::unloadTexture(TextureHandle texture)
{
    auto it = _textures.find(texture);
    if (it == _textures.end()) return;
    UnloadTexture(it->second);
    _textures.erase(it);
}

void RaylibRenderer::unloadFont(FontHandle font)
{
    auto it = _fonts.find(font);
    if (it == _fonts.end()) return;
    UnloadFont(it->second);
    _fonts.erase(it);
}

Vector2f RaylibRenderer::worldToScreen(Vector3f worldPos) const
{
    ::Vector3 v = { worldPos.x, worldPos.y, worldPos.z };
    ::Vector2 s = GetWorldToScreen(v, _currentCamera);
    return { s.x, s.y };
}

Vector3f RaylibRenderer::screenToWorldRay(Vector2f screenPos) const
{
    ::Ray ray = GetMouseRay({screenPos.x, screenPos.y}, _currentCamera);
    return { ray.direction.x, ray.direction.y, ray.direction.z };
}

RayCollisionData RaylibRenderer::checkRayMeshCollision(
    Vector3f rayOrigin, Vector3f rayDirection,
    MeshHandle mesh, Vector3f meshPosition, Vector3f meshScale)
{
    RayCollisionData result{};
    auto it = _models.find(mesh);
    if (it == _models.end()) return result;

    ::Ray ray = { {rayOrigin.x, rayOrigin.y, rayOrigin.z},
                  {rayDirection.x, rayDirection.y, rayDirection.z} };
    ::Matrix scale     = MatrixScale(meshScale.x, meshScale.y, meshScale.z);
    ::Matrix translate = MatrixTranslate(meshPosition.x, meshPosition.y, meshPosition.z);
    ::Matrix transform = MatrixMultiply(scale, translate);
    ::RayCollision col = GetRayCollisionMesh(ray, it->second.meshes[0], transform);

    result.hasHit      = col.hit;
    result.distance    = col.distance;
    result.hitPosition = { col.point.x,  col.point.y,  col.point.z };
    result.hitUV       = { col.normal.x, col.normal.y };
    return result;
}

} // namespace graphic::raylib
