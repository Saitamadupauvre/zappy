#pragma once

#include "graphic/IRenderer.hpp"
#include "graphic/Types.hpp"
#include "graphic/Matrix4x4.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include <array>
#include <string>
#include <vector>

namespace zappy {

// Batches all resource gem/food draws into one DrawMeshInstanced call per
// resource type per mesh, instead of one DrawModel call per entity.
class ResourceInstanceBatch {
public:
    static constexpr int NUM_TYPES = 7;

    struct TypeBatch {
        graphic::ModelHandle          model{0};
        graphic::Color4b              tint;
        std::vector<graphic::Color4b> meshTints;
        std::vector<std::string>      meshShaders;
        bool                          initialized = false;
        std::vector<graphic::Matrix4x4> transforms;
    };

    void markDirty()  { _dirty = true; }
    bool isDirty() const { return _dirty; }

    // Collects transforms of visible resource entities per type.
    void rebuild(const EntityManager& em);

    // Issues instanced draw calls for each type.
    void draw(graphic::IRenderer& renderer);

private:
    std::array<TypeBatch, NUM_TYPES> _batches;
    bool _dirty = true;
};

} // namespace zappy
