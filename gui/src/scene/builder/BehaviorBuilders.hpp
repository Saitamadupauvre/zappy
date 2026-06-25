#pragma once

#include <memory>
#include <functional>

namespace graphic {
    class Entity;
}

class EntityBuilder;

class TransformBuilder;
class MovementBuilder;
class InteractionBuilder;
class DrawableBuilder;
class PlayerBuilder;
class ResourceBuilder;
class EggBuilder;
class TagBuilder;
class HudBuilder;

using EntityPtr = std::shared_ptr<graphic::Entity>;