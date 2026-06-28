#include "ProcessSpawner.hpp"

#ifdef __unix__
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#endif

namespace zappy {

int ProcessSpawner::spawn(const std::string& cmd)
{
#ifdef __unix__
    pid_t pid = fork();
    if (pid == 0) {
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        execlp("sh", "sh", "-c", cmd.c_str(), nullptr);
        _exit(1);
    }
    int ipid = static_cast<int>(pid);
    if (ipid > 0 && _onSpawn)
        _onSpawn(ipid);
    return ipid;
#else
    (void)cmd;
    return -1;
#endif
}

} // namespace zappy
