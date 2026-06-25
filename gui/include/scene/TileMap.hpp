#pragma once

#include "graphic/Types.hpp"
#include "scene/layout/IMapLayout.hpp"
#include <vector>
#include <cstdlib>

namespace zappy {

class TileMap
{
public:
    void build(const IMapLayout& layout, int worldW, int worldH)
    {
        _w = worldW;
        _h = worldH;
        _tilePos.resize(worldW * worldH);
        _standPos.resize(worldW * worldH);
        _upAt.resize(worldW * worldH);
        _fwdAt.resize(worldW * worldH);
        for (int y = 0; y < worldH; ++y) {
            for (int x = 0; x < worldW; ++x) {
                int   i   = _idx(x, y);
                auto  base = layout.tilePos(x, y, worldW, worldH);
                auto  up   = layout.upAt(x, y, worldW, worldH);
                float sy   = layout.standY(x, y, worldW, worldH);
                _tilePos[i]  = base;
                _upAt[i]     = up;
                _fwdAt[i]    = layout.forwardAt(x, y, worldW, worldH);
                _standPos[i] = {base.x + up.x * sy,
                                base.y + up.y * sy,
                                base.z + up.z * sy};
            }
        }
    }

    void clear()
    {
        _tilePos.clear();
        _standPos.clear();
        _upAt.clear();
        _fwdAt.clear();
        _w = 0;
        _h = 0;
    }

    graphic::Vector3f tilePos(int x, int y) const
    {
        int i = _idx(x, y);
        return (i >= 0 && i < static_cast<int>(_tilePos.size())) ? _tilePos[i] : graphic::Vector3f{0,0,0};
    }

    graphic::Vector3f standPos(int x, int y) const
    {
        int i = _idx(x, y);
        return (i >= 0 && i < static_cast<int>(_standPos.size())) ? _standPos[i] : graphic::Vector3f{0, 0, 0};
    }

    graphic::Vector3f tileUp(int x, int y) const
    {
        int i = _idx(x, y);
        return (i >= 0 && i < static_cast<int>(_upAt.size())) ? _upAt[i] : graphic::Vector3f::up();
    }

    graphic::Vector3f tileForward(int x, int y) const
    {
        int i = _idx(x, y);
        return (i >= 0 && i < static_cast<int>(_fwdAt.size())) ? _fwdAt[i] : graphic::Vector3f::forward();
    }

    // Returns true when the shortest path between two tiles crosses an edge (wrap-around).
    bool isWrapMove(int fromX, int fromY, int toX, int toY) const
    {
        int dx = std::abs(toX - fromX);
        int dy = std::abs(toY - fromY);
        // A normal forward step is at most 1 tile in any direction.
        // Anything further is an edge-wrap or server correction → teleport.
        return dx > 1 || dy > 1;
    }

    int  width()  const { return _w; }
    int  height() const { return _h; }
    bool built()  const { return _w > 0 && _h > 0; }

private:
    int _idx(int x, int y) const { return y * _w + x; }

    std::vector<graphic::Vector3f> _tilePos;
    std::vector<graphic::Vector3f> _standPos;
    std::vector<graphic::Vector3f> _upAt;
    std::vector<graphic::Vector3f> _fwdAt;
    int _w = 0;
    int _h = 0;
};

} // namespace zappy
