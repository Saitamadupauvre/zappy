#include "core/GameEngine.hpp"

int main(int argc, const char **argv)
{
    zappy::GameEngine engine(argc, argv);
    engine.run();

    return engine.getStatus();
}
