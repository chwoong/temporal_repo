#include <cstdlib>
#include <ctime>
#include "src/game/Game.hpp"

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    Game game;
    game.run();
    return 0;
}
