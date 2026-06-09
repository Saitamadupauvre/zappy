#pragma once

#include "behavior/ABehavior.hpp"
#include "graphic/Types.hpp"

namespace behavior {

class TransformBehavior : public ABehavior {
public:
    TransformBehavior() = default;
    ~TransformBehavior() override = default;

    // Contrat ABehavior
    void onUpdate(graphic::Entity& owner, float deltaTime) override;

    void setPosition(const graphic::Vector3f& pos);
    void setRotation(const graphic::Vector3f& rot);
    void setScale(const graphic::Vector3f& scale);

    const graphic::Vector3f& getPosition() const { return _position; }
    const graphic::Vector3f& getRotation() const { return _rotation; }
    const graphic::Vector3f& getScale() const    { return _scale; }

    const graphic::Matrix4x4& getMatrix();

private:
    void updateMatrix();

    graphic::Vector3f _position{0.0f, 0.0f, 0.0f};
    graphic::Vector3f _rotation{0.0f, 0.0f, 0.0f};
    graphic::Vector3f _scale{1.0f, 1.0f, 1.0f};
    
    graphic::Matrix4x4 _matrix;
    bool _needsUpdate = true;
};

} // namespace behavior