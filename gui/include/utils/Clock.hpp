#pragma once

#include <chrono>

namespace zappy::utils {

class Clock
{
public:
    using clock_type = std::chrono::steady_clock;
    using time_point = clock_type::time_point;
    using duration = clock_type::duration;

    Clock() noexcept
        : _start(clock_type::now())
        , _last(_start)
        , _current(_start)
    {
    }

    explicit Clock(time_point start) noexcept
        : _start(start)
        , _last(start)
        , _current(start)
    {
    }

    void reset() noexcept
    {
        _start = clock_type::now();
        _last = _start;
        _current = _start;
    }

    void update() noexcept
    {
        _last = _current;
        _current = clock_type::now();
    }

    [[nodiscard]] time_point start() const noexcept { return _start; }
    [[nodiscard]] time_point last() const noexcept { return _last; }
    [[nodiscard]] time_point current() const noexcept { return _current; }

    [[nodiscard]] duration delta() const noexcept { return _current - _last; }
    [[nodiscard]] duration elapsed() const noexcept { return _current - _start; }

    [[nodiscard]] long long deltaMs() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(delta()).count();
    }

    [[nodiscard]] long long elapsedMs() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed()).count();
    }

    [[nodiscard]] double deltaSec() const noexcept
    {
        return std::chrono::duration<double>(delta()).count();
    }

    [[nodiscard]] double elapsedSec() const noexcept
    {
        return std::chrono::duration<double>(elapsed()).count();
    }

    [[nodiscard]] bool deltaMsAtLeast(long long ms) const noexcept
    {
        return deltaMs() >= ms;
    }

    [[nodiscard]] bool elapsedMsAtLeast(long long ms) const noexcept
    {
        return elapsedMs() >= ms;
    }

    [[nodiscard]] bool deltaSecAtLeast(double sec) const noexcept
    {
        return deltaSec() >= sec;
    }

    [[nodiscard]] bool elapsedSecAtLeast(double sec) const noexcept
    {
        return elapsedSec() >= sec;
    }

private:
    time_point _start;
    time_point _last;
    time_point _current;
};

} // namespace zappy::utils
