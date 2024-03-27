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
        bool launchServer = ret == gameAction_launchServer;
        cout << &gr << endl;
        Game game(&gr, server, username, password, launchServer);
        while (game.looping()) {
            game.update();
        }
        game.end();

        cout << "quit reason: " << game.getQuitInfo() << endl;
    }
    return 0;
}

// ce faila requesta se enkrat, silly window syndrome
// TO-DO: domnu je sel offset cez recvbuflen na Game::process_init() (client), (1460,1525) in pol pumpa napacne podatke not

// server posle 5168
/*
Unable to connect to server!
3
Unable to connect to server!
ConnectSocket in non-blocking mode
Bytes Sent: 1
0
000000f06190fa38
ConnectSocket in non-blocking mode
Bytes Sent: 6
init completed
Client ran...
L click at 151, 235
dt capped
- request sent
- update all requested...
Bytes Sent: 2
- request sent
- update all requested...
Bytes Sent: 2
Data recieved!
BUFFLEN = 4380
RESETAM WORLD!!!
generating planets, seed=1234, count=10
points len = 18
virt len=8
0,1,2,3,4,5,6,7,
virt len=8
18,19,20,21,22,23,24,25,
lineObst len = 0
links len = 34
muscles len = 0
lineObst len = 0
rocketThr len = 16
fuelCont len = 2
textures len = 2
media/astronaut.png
0,1,8: 0.333333,0.259259;0,0.62963;0.5,0.5,
1,2,8: 0,0.62963;0.333333,1;0.5,0.5,
2,3,8: 0.333333,1;0.666667,1;0.5,0.5,
E: @ PhTexture::push_indicie: norms out of bounds!!!
3,4,8: 0.666667,2.88675e-164;-2.8701e+174,-2.8701e+174;-2.8701e+174,-2.8701e+174,
32758,-467192472,32758: 6.95142e-310,6.95142e-310;6.95142e-310,6.95142e-310;6.95142e-310,6.95142e-310,
E: @ PhTexture::push_indicie: norms out of bounds!!!
*/