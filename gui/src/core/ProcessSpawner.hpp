#pragma once

#include <functional>
#include <string>

namespace zappy {

class ProcessSpawner {
public:
    using OnSpawn = std::function<void(int pid)>;

    void setOnSpawn(OnSpawn cb) { _onSpawn = std::move(cb); }

    // Returns pid > 0 on success, -1 on failure or non-unix
    int spawn(const std::string& cmd);

private:
    OnSpawn _onSpawn;
};

} // namespace zappy
