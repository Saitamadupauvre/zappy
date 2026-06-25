#pragma once

namespace zappy {

class AnimationClock
{
public:
    void  setTimeUnit(int t)          { _t = static_cast<float>(t > 0 ? t : 1); }
    float getTimeUnit()         const { return _t; }

    float toSeconds(float ticks) const { return ticks / _t; }

    float moveDuration()         const { return toSeconds(7.0f); }
    float incantationDuration()  const { return toSeconds(300.0f); }
    float forkDuration()         const { return toSeconds(42.0f); }

private:
    float _t = 100.0f;
};

} // namespace zappy
