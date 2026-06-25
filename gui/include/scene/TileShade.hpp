#pragma once

namespace zappy {

// Per-tile darkness factor in [0,1], replacing the old gray checkerboard.
// Single source of truth shared by the ground texture and the grass tint so
// the blades darken exactly where the tile darkens.
inline float tileDarkness(int x, int y)
{
    return ((x + y) % 2 == 0) ? 0.45f : 1.0f;
}

} // namespace zappy
