#include "ModelDrawableBehavior.hpp"
#include <cmath>

namespace behavior {

ModelDrawableBehavior::ModelDrawableBehavior(graphic::IRenderer& renderer,
                                             graphic::ModelHandle model,
                                             graphic::Color4b tint,
                                             bool ownsModel)
    : _renderer(&renderer), _model(model), _tint(tint), _ownsModel(ownsModel)
{
    _mesh = _renderer->meshFromModel(_model);
}

ModelDrawableBehavior::~ModelDrawableBehavior()
{
    if (_renderer) {
        _renderer->unloadMesh(_mesh);
        if (_ownsModel)
            _renderer->unloadModel(_model);
    }
}

void ModelDrawableBehavior::setRotationOffset(const graphic::Vector3f& eulerDeg)
{
    constexpr float DEG2RAD = M_PI / 180.0f;
    _offset = graphic::Matrix4x4::RotationEuler(eulerDeg.x * DEG2RAD,
                                                eulerDeg.y * DEG2RAD,
                                                eulerDeg.z * DEG2RAD);
    _hasOffset = true;
}

void ModelDrawableBehavior::draw(graphic::IRenderer& renderer,
                                 const graphic::Matrix4x4& transform)
{
    if (!_visible) return;
    // Row-vector convention: offset applied in local space, before the entity's
    // scale/rotation/translation.
    graphic::Matrix4x4 finalTransform = _hasOffset ? _offset * transform : transform;
    renderer.drawModel(graphic::ModelDrawParams{
        .model       = _model,
        .transform   = finalTransform,
        .tint        = _tint,
        .meshTints   = _meshTints,
        .meshShaders = _meshShaders,
    });
}

} // namespace behavior
