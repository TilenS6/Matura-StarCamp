#include <iostream>
#include "game/game.h"

using namespace std;

int main(int argc, char *argv[]) {
    Game game;

    while (game.looping()) {
        game.update();
    }
    return 0;
}