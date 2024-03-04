#include <iostream>
#include "mainmenu/mainmenu.cpp"

using namespace std;
int main(int argc, char *argv[]) {
    GameRenderer gr;
    gr.init();
    while (true) {
        int ret = qeMenu_game;
        while (ret != gameAction_play) {
            if (ret == qeMenu_game)
                ret = mainmenu_game(&gr);
            else if (ret == qeMenu_settings)
                ret = mainmenu_settings(&gr);
            else if (ret == qeMenu_info)
                ret = mainmenu_info(&gr);
            else
                ret = qeMenu_game;

            if (ret == -1) return 0;
        }

        // PLAY
        Game game(&gr);
        while (game.looping()) {
            game.update();
        }
    }
    return 0;
}