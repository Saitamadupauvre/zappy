#pragma once

#include "scene/builder/BehaviorBuilders.hpp"
#include "graphic/Types.hpp"

#include <string>

namespace graphic { class IRenderer; }

// Visual expert: mesh and text drawables plus visibility.
class DrawableBuilder {
    public:
        DrawableBuilder(EntityBuilder& owner, EntityPtr entity);

        EntityBuilder& mesh(graphic::MeshHandle mesh,
                            graphic::TextureHandle texture,
                            graphic::Color4b tint = graphic::Color4b::white());
        EntityBuilder& model(graphic::IRenderer& renderer,
                             graphic::ModelHandle model,
                             graphic::Color4b tint = graphic::Color4b::white(),
                             const graphic::Vector3f& rotationOffset = {0.0f, 0.0f, 0.0f},
                             bool ownsModel = true);
        EntityBuilder& text(const std::string& text,
                            float size = 18.0f,
                            graphic::Color4b color = {255, 255, 255, 255},
                            graphic::Anchor anchor = graphic::Anchor::Center);
        EntityBuilder& meshVisible(bool visible);
        EntityBuilder& textVisible(bool visible);

    private:
        EntityBuilder& _owner;
        EntityPtr      _entity;
};
