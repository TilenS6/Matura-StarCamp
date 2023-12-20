#include <iostream>
#include "game/game.h"

using namespace std;

// TODO neki general multithreading: input events -- physics updating(?) -- rendering -- multiplayer handeling (?)

int main(int argc, char *argv[]) {
    Game game;

    while (game.looping()) {
        game.update();
    }
    return 0;
}