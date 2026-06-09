#include "TransformBehavior.hpp"

namespace behavior {

void TransformBehavior::onUpdate(graphic::Entity& owner, float deltaTime) {
    if (_needsUpdate) {
        updateMatrix();
    }
}

void TransformBehavior::setPosition(const graphic::Vector3f& pos) {
    if (_position.x != pos.x || _position.y != pos.y || _position.z != pos.z) {
        _position = pos;
        _needsUpdate = true;
    }
}

void TransformBehavior::setRotation(const graphic::Vector3f& rot) {
    if (_rotation.x != rot.x || _rotation.y != rot.y || _rotation.z != rot.z) {
        _rotation = rot;
        _needsUpdate = true;
    }
}

void TransformBehavior::setScale(const graphic::Vector3f& scale) {
    if (_scale.x != scale.x || _scale.y != scale.y || _scale.z != scale.z) {
        _scale = scale;
        _needsUpdate = true;
    }
}

const graphic::Matrix4x4& TransformBehavior::getMatrix() {
    if (_needsUpdate) {
        updateMatrix();
    }
    return _matrix;
}

void TransformBehavior::updateMatrix() {
    graphic::Matrix4x4 matTrans = graphic::Matrix4x4::Translation(_position.x, _position.y, _position.z);
    graphic::Matrix4x4 matScale = graphic::Matrix4x4::Scale(_scale.x, _scale.y, _scale.z);
    graphic::Matrix4x4 matRot   = graphic::Matrix4x4::RotationY(_rotation.y);

    _matrix = matScale * matRot * matTrans;
    
    _needsUpdate = false;
}

} // namespace behavior