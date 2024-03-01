#include <iostream>
#include "game/game.h"
#include "mainmenuog/mainmenuog.h"

using namespace std;
int main(int argc, char *argv[]) {
    GameRenderer rend;
    rend.init();

    MainMenu menu;
    string opt[] = {
        "test1",
        "test2",
        "test3",
        "test4",
        "blbllblablsblabl",
    };
    
    int n = 5;
    while (true) {
        cout << menu.chose(rend.cam.r, opt, n, "Test") << endl;
    }

    Mouse m;
    while (!rend.basicEvents()) {
        uint8_t m_ev = m.update();

        rend.clear();

        rend.represent();
    }

    /*
    Game game;

    while (game.looping()) {
        game.update();
    }
    */
    return 0;
}