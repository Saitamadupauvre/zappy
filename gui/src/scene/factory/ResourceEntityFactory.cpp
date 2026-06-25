#include "ResourceEntityFactory.hpp"
#include "entity/Entity.hpp"
#include <cmath>
#include <cstdint>
#include "core/manager/entity/EntityManager.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include <iostream>

namespace zappy {

static constexpr graphic::Color4b RESOURCE_COLORS[7] = {
    {  50, 200,  50, 255},
    { 100, 200, 255, 255},
    { 255, 150,  50, 255},
    { 180, 100, 255, 255},
    { 255, 220,  50, 255},
    { 255, 120, 180, 255},
    { 255,  60,  60, 255},
};

static constexpr graphic::Color4b STONE_TINT = {120, 120, 120, 255};

static constexpr float RESOURCE_SCALES[7] = {
    1.0f, 0.85f, 0.9f, 0.95f, 1.0f, 1.05f, 1.2f
};

static uint32_t pcgHash(uint32_t v)
{
    v ^= v >> 17; v *= 0xbf58476du; v ^= v >> 31; v *= 0x94d049bbu; v ^= v >> 17;
    return v;
}

static graphic::Vector3f resourceOffset(int x, int y, int slot)
{
    uint32_t seed = static_cast<uint32_t>(x) * 2654435761u
                  ^ static_cast<uint32_t>(y) * 2246822519u
                  ^ static_cast<uint32_t>(slot) * 3266489917u;
    uint32_t hx = pcgHash(seed);
    uint32_t hz = pcgHash(seed ^ 0xdeadbeef);
    // restrict to [-0.45, +0.45] so resources don't overlap tile edges
    float ox = (static_cast<float>(hx) / 4294967295.0f) * 0.9f - 0.45f;
    float oz = (static_cast<float>(hz) / 4294967295.0f) * 0.9f - 0.45f;
    return {ox, 0.15f, oz};
}

static constexpr float CRYSTAL_BASE_SCALE    = 0.18f;
static constexpr float CRYSTAL_MODEL_MIN_Y   = -0.17476442f;
static constexpr float FOOD_BASE_SCALE       = 0.18f;
static constexpr float FOOD_MODEL_MIN_Y      = -0.02074003f;

ResourceEntityFactory::~ResourceEntityFactory()
{
    if (_renderer) {
        _renderer->unloadModel(_model);
        _renderer->unloadModel(_foodModel);
    }
}

void ResourceEntityFactory::init(graphic::IRenderer& renderer,
                                  graphic::IMeshFactory& factory)
{
    (void)factory;
    _renderer = &renderer;
    _model     = renderer.loadModel(MODEL_PATH);
    _foodModel = renderer.loadModel(FOOD_MODEL_PATH);
}

void ResourceEntityFactory::spawnAll(EntityManager& em, int x, int y, int worldW,
                                      const graphic::Vector3f& tileBase,
                                      const graphic::Vector3f& upAt) const
{
    graphic::Vector3f up  = upAt.normalized();
    graphic::Vector3f ref = (std::abs(up.y) < 0.9f) ? graphic::Vector3f{0.f, 1.f, 0.f} : graphic::Vector3f{1.f, 0.f, 0.f};
    graphic::Vector3f right   = ref.cross(up).normalized();
    graphic::Vector3f forward = up.cross(right);

    for (int i = 0; i < 7; ++i) {
        bool isFood = (i == 0);
        float baseScale  = isFood ? FOOD_BASE_SCALE    : CRYSTAL_BASE_SCALE;
        float modelMinY  = isFood ? FOOD_MODEL_MIN_Y   : CRYSTAL_MODEL_MIN_Y;
        float s          = RESOURCE_SCALES[i] * baseScale;
        graphic::Vector3f ov = resourceOffset(x, y, i);
        float lift       = -modelMinY * s;
        graphic::Vector3f pos = {
            tileBase.x + right.x*ov.x + up.x*lift + forward.x*ov.z,
            tileBase.y + right.y*ov.x + up.y*lift + forward.y*ov.z,
            tileBase.z + right.z*ov.x + up.z*lift + forward.z*ov.z
        };

        graphic::ModelHandle handle = isFood ? _foodModel : _model;
        graphic::Color4b tint = isFood ? graphic::Color4b{255, 255, 255, 255} : RESOURCE_COLORS[i];

        graphic::EntityID eid = resourceId(x, y, worldW, i);
        em.registerTileListener(x, y, eid);
        auto entity = EntityBuilder(em, eid, "resource")
            .transform().position(pos)
            .transform().scale(s)
            .transform().orientation(up)
            .interaction().outline()
            .interaction().selectableOutline()
            .interaction().hoverScale(s)
            .interaction().onClick([x, y, i]([[maybe_unused]] graphic::Entity& e) {
                std::cout << "Ressource [" << x << "," << y << "] type " << i << " cliquée !" << std::endl;
            })
            .drawable().model(*_renderer, handle, tint, {0.0f, 0.0f, 0.0f}, false)
            .drawable().meshVisible(false)
            .resource().data(x, y, i)
            .build();

        if (!isFood) {
            if (auto d = entity->getBehavior<behavior::ModelDrawableBehavior>()) {
                d->setMeshTints({STONE_TINT, {RESOURCE_COLORS[i].r, RESOURCE_COLORS[i].g, RESOURCE_COLORS[i].b, 165}});
                d->setMeshShaders({"", "crystal"});
            }
        }
    }
}

void ResourceEntityFactory::clearAll(EntityManager& em, int worldW, int worldH) const
{
    for (int y = 0; y < worldH; ++y)
        for (int x = 0; x < worldW; ++x)
            for (int i = 0; i < 7; ++i)
                em.removeEntity(resourceId(x, y, worldW, i));
    em.clearTileListeners();
}
} // namespace zappy
