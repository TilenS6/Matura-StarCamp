#include <iostream>
#include <thread>
#include "mainmenu/mainmenu.cpp"

using namespace std;

#define debug(x) std::cout << #x << " = " << x << std::endl;

string server = "127.0.0.1";
string checkConnection_report = "";
string username = "a", password = "a";
bool pingingServer = true, pingingServer_accepted = true;
bool terminatePing = false;
bool terminatePing_accepted = false;

int main(int argc, char *argv[]) {
    terminatePing = false;
    GameRenderer gr;
    gr.init();
    while (true) {
        terminatePing = false;
        pingingServer = true;
        int ret = qeMenu_game;
        thread thr = thread(checkConnection);

        while (ret != gameAction_play && ret != gameAction_launchServer) {
            if (ret == qeMenu_game)
                ret = mainmenu_game(&gr);
            else if (ret == qeMenu_settings)
                ret = mainmenu_settings(&gr);
            else if (ret == qeMenu_info)
                ret = mainmenu_info(&gr);
            else
                ret = qeMenu_game;

            if (ret == -1) {
                int i = 0;
                terminatePing = true;
                while (!terminatePing_accepted) {
                    gr.basicEvents();
                    SDL_Rect rct = {rand() % gr.cam.w, rand() % gr.cam.h, 10, 10};
                    SDL_SetRenderDrawColor(gr.cam.r, 0, 0, 0, 255);
                    SDL_RenderFillRect(gr.cam.r, &rct);
                    gr.represent();
                    ++i;
                }
                thr.join();
                return 0;
            }

            if (ret == gameAction_addUser) {
                if (new_username.length() >= 3 && new_password.length() >= 3) {
                    LoginEntry tmp = {new_username, new_password, {0, 0}};
                    login.push_back(tmp);
                    ret = qeMenu_settings;
                }
            }
        }
        terminatePing = true;

        int i = 0;
        while (!terminatePing_accepted) {
            gr.basicEvents();
            SDL_Rect rct = {rand() % gr.cam.w, rand() % gr.cam.h, 10, 10};
            SDL_SetRenderDrawColor(gr.cam.r, 0, 0, 0, 255);
            SDL_RenderFillRect(gr.cam.r, &rct);
            gr.represent();
            ++i;
        }
        thr.join();

        // PLAY
        SDL_ShowCursor(SDL_ENABLE);
        bool launchServer = (ret == gameAction_launchServer);
        Game game(&gr, server, username, password, launchServer);
        while (game.looping()) {
            game.update();
        }
        game.end();

        cout << "quit reason: " << game.getQuitInfo() << endl;
    }
    return 0;
}