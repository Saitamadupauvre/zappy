#include "TransformBehavior.hpp"

namespace behavior {

void TransformBehavior::onUpdate([[maybe_unused]] graphic::Entity& owner, [[maybe_unused]] float deltaTime) {
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

void TransformBehavior::setOrientation(const graphic::Vector3f& up, float yawDeg) {
    constexpr float DEG2RAD = M_PI / 180.0f;
    _customRot    = graphic::Matrix4x4::RotationAlignUp(up, yawDeg * DEG2RAD);
    _useCustomRot = true;
    _needsUpdate  = true;
}

void TransformBehavior::setOrientation(const graphic::Vector3f& up,
                                       const graphic::Vector3f& forward, float yawDeg) {
    constexpr float DEG2RAD = M_PI / 180.0f;
    _customRot    = graphic::Matrix4x4::OrientationFromUpForward(up, forward, yawDeg * DEG2RAD);
    _useCustomRot = true;
    _needsUpdate  = true;
}

void TransformBehavior::updateMatrix() {
    constexpr float DEG2RAD = M_PI / 180.0f;
    graphic::Matrix4x4 matTrans = graphic::Matrix4x4::Translation(_position.x, _position.y, _position.z);
    graphic::Matrix4x4 matScale = graphic::Matrix4x4::Scale(_scale.x, _scale.y, _scale.z);
    graphic::Matrix4x4 matRot   = _useCustomRot
        ? _customRot
        : graphic::Matrix4x4::RotationY(_rotation.y * DEG2RAD);

    // Row-vector convention: v * (S * R * T) = scale, rotate around local origin, then translate.
    _matrix = matScale * matRot * matTrans;

    _needsUpdate = false;
}

} // namespace behavior