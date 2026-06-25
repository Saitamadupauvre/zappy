#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class TransformBehavior : public ABehavior {
public:
    TransformBehavior() = default;
    ~TransformBehavior() override = default;

    void onUpdate(graphic::Entity& owner, float deltaTime) override;

    void setPosition(const graphic::Vector3f& pos);
    void setRotation(const graphic::Vector3f& rot);
    void setScale(const graphic::Vector3f& scale);
    void setOrientation(const graphic::Vector3f& up, float yawDeg = 0.0f);
    void setOrientation(const graphic::Vector3f& up, const graphic::Vector3f& forward,
                        float yawDeg = 0.0f);

    const graphic::Vector3f& getPosition() const { return _position; }
    const graphic::Vector3f& getRotation() const { return _rotation; }
    const graphic::Vector3f& getScale() const    { return _scale; }

    graphic::Vector3f getUp() const {
        if (!_useCustomRot) return graphic::Vector3f::up();
        return { _customRot.m[4], _customRot.m[5], _customRot.m[6] };
    }
    graphic::Vector3f getForward() const {
        if (!_useCustomRot) return graphic::Vector3f::forward();
        return { _customRot.m[8], _customRot.m[9], _customRot.m[10] };
    }

    const graphic::Matrix4x4& getMatrix();

private:
    void updateMatrix();

    graphic::Vector3f  _position{0.0f, 0.0f, 0.0f};
    graphic::Vector3f  _rotation{0.0f, 0.0f, 0.0f};
    graphic::Vector3f  _scale{1.0f, 1.0f, 1.0f};
    graphic::Matrix4x4 _customRot   = graphic::Matrix4x4::Identity();
    bool               _useCustomRot = false;

    graphic::Matrix4x4 _matrix;
    bool _needsUpdate = true;
};

} // namespace behavior