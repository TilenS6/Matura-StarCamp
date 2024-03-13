#include <iostream>
#include "game/game.h"
#include "menu/menu.h"

#include <thread>
#include "netagent/netagent.h"
#include "netagent/netstds.cpp"

extern string server;
extern string checkConnection_report;
extern string username, password;
extern bool pingingServer;
extern bool pingingServer_accepted;
extern bool terminatePing;
extern bool terminatePing_accepted;
extern Text *txtStatusP;

enum qeMenu {
    qeMenu_game = -2,
    qeMenu_settings = -3,
    qeMenu_info = -4,
};

enum gameAction {
    gameAction_play,
    gameAction_launchServer,
    gameAction_addUser,
};

/* login packet
 ! NO HEADER

 ---- username
 uint16_t len
 char c0, c1, c2...
 ---- password
 uint16_t len
 char c0, c1, c2...

*/
void checkConnection() {
    while (true) {
        NetClient client;
        terminatePing_accepted = terminatePing;
        if (terminatePing_accepted)
            return;

        pingingServer_accepted = pingingServer;
        if (pingingServer_accepted) {
            checkConnection_report += "...";
            client.init(server);
            switch (client.getConnectionStatus()) {
            case getConnectionStatus_NO_HOST:
                checkConnection_report = "cannot resolve IP address";
                break;

            case 0:
                checkConnection_report = "OK";
                {
                    char dt[] = {0};
                    Timer t;
                    t.interval();
                    client.sendData(dt, 1);

                    while (t.getTime() < 5 && client.recieveData() == recieveData_NO_NEW_DATA)
                        asm("nop");

                    double ping = t.interval();
                    if (client.recvbuflen != 1 || client.recvbuf[0] != 1) {
                        checkConnection_report = "OK, update required";
                    } else {
                        string str = "OK, ";
                        str += to_string((int)(ping * 1000));
                        str += "ms";
                        checkConnection_report = str;
                    }
                }
                client.closeConnection();
                break;

            case getConnectionStatus_ERR:
            default:
                checkConnection_report = "connection error";
                break;
            }
        }

        Sleep(500);
    }
}

int mainmenu_game(GameRenderer *gr) {
    Menu menu;
    string opt[] = {
        "Play",
        "Exit",
    };
    int n = 2;
    string qeMenu[] = {"Game", "Settings", "Info"};
    int qeN = 3;
    string title = "StarCamp";

    MenuRect rect;
    rect.w = 200;
    rect.h = 200;
    rect.x = gr->cam.w - 100 - rect.w;
    rect.y = 50 * 3 * 1.3;

    Text txt;
    txt.changeText(gr->cam.r, "Server status:", 24);
    txt.move(rect.x + 10, rect.y + 10);

    Text txtStatus;
    txtStatus.changeText(gr->cam.r, checkConnection_report, 16);
    txtStatus.linkText(&checkConnection_report);
    txtStatus.move(rect.x + 10, rect.y + 10 + 30);

    Button btn;
    btn.move(rect.x + 10, rect.y + 10 + 30 + 50, 150, 40);
    btn.changeText(gr->cam.r, "Refresh");

    Button btnStartServer;
    btnStartServer.move(rect.x, rect.y + 10 + 30 + 50 + 50, rect.w, 40);
    btnStartServer.changeText(gr->cam.r, "Launch server");
    btnStartServer.changeStyle(1);

    Text txtUsername, txtPassword;
    txtUsername.changeText(gr->cam.r, "Username: ", 24);
    txtUsername.move(70, 5.5 * 50 * 1.3);
    txtPassword.changeText(gr->cam.r, "Password: ", 24);
    txtPassword.move(70, 6 * 50 * 1.3);

    Input inputUsername, inputPassword;
    inputUsername.x = 220;
    inputUsername.y = 5.5 * 50 * 1.3;
    inputUsername.w = 200;
    inputUsername.h = 30;
    inputUsername.linkText(gr->cam.r, &username);

    inputPassword.x = 210;
    inputPassword.y = 6 * 50 * 1.3;
    inputPassword.w = 200;
    inputPassword.h = 30;
    inputPassword.linkText(gr->cam.r, &password, true);

    int ret = menu.chose(gr->cam.r, opt, n, title, qeMenu, qeN, 0,                                                                    // static options
                         &rect, &txt, &txtStatus, &btn, &btnStartServer, &txtUsername, &txtPassword, &inputUsername, &inputPassword); // added features

    txt.destroy();
    txtStatus.destroy();
    txtUsername.destroy();
    txtPassword.destroy();
    inputUsername.destroy();
    inputPassword.destroy();

    cout << ret << endl;
    if (ret == 0)
        return gameAction_play; // Play
    if (ret == 1)
        return -1;            // Exit
    if (ret == 2 || ret == 5) // Refresh
        return qeMenu_game;
    if (ret == 3)
        return qeMenu_settings;
    if (ret == 4)
        return qeMenu_info;

    if (ret == 6)
        return gameAction_launchServer;
    return ret;
}

//----------------------------------------------------------------------------------------------------
string new_username, new_password;
int mainmenu_settings(GameRenderer *gr) {
    new_username = "";
    new_password = "";

    Menu menu;
    string opt[] = {

    };
    int n = 0;
    string qeMenu[] = {"Game", "Settings", "Info"};
    int qeN = 3;
    string title = "StarCamp";

    MenuRect rect;
    rect.w = 200;
    rect.h = 200;
    rect.x = gr->cam.w - 100 - rect.w;
    rect.y = 50 * 3 * 1.3;

    Text txt;
    txt.changeText(gr->cam.r, "Server status:", 24);
    txt.move(rect.x + 10, rect.y + 10);

    Text txtStatus;
    txtStatus.changeText(gr->cam.r, checkConnection_report, 16);
    txtStatus.linkText(&checkConnection_report);
    txtStatus.move(rect.x + 10, rect.y + 10 + 30);

    Button btn;
    btn.move(rect.x + 10, rect.y + 10 + 30 + 30, 150, 40);
    btn.changeText(gr->cam.r, "Refresh");

    Text txtIp;
    txtIp.changeText(gr->cam.r, "IP:", 24);
    txtIp.move(70, 50 * 3 * 1.3);

    Input inputIp;
    inputIp.x = 150;
    inputIp.y = 50 * 3 * 1.3;
    inputIp.w = 200;
    inputIp.h = 30;
    inputIp.linkText(gr->cam.r, &server);

    Text txtUsername, txtPassword;
    txtUsername.changeText(gr->cam.r, "Username: ", 24);
    txtUsername.move(70, 5.5 * 50 * 1.3);
    txtPassword.changeText(gr->cam.r, "Password: ", 24);
    txtPassword.move(70, 6 * 50 * 1.3);

    Input inputUsername, inputPassword;
    inputUsername.x = 220;
    inputUsername.y = 5.5 * 50 * 1.3;
    inputUsername.w = 200;
    inputUsername.h = 30;
    inputUsername.linkText(gr->cam.r, &new_username);

    inputPassword.x = 210;
    inputPassword.y = 6 * 50 * 1.3;
    inputPassword.w = 200;
    inputPassword.h = 30;
    inputPassword.linkText(gr->cam.r, &new_password, true);

    Button btnAddUser;
    btnAddUser.move(70, 6 * 50 * 1.3 + 50, 200, 40);
    btnAddUser.changeText(gr->cam.r, "Add user");

    // thread thr = thread(checkConnection, &txtStatus);

    int ret = menu.chose(gr->cam.r, opt, n, title, qeMenu, qeN, 1,                                                                                  // static options
                         &rect, &txt, &txtStatus, &btn, &txtIp, &inputIp, &btnAddUser, &txtUsername, &txtPassword, &inputUsername, &inputPassword); // added features

    txt.destroy();
    txtStatus.destroy();
    btn.destroy();
    inputIp.destroy();
    btnAddUser.destroy();

    txtUsername.destroy();
    txtPassword.destroy();
    inputUsername.destroy();
    inputPassword.destroy();

    if (ret == 0)
        return qeMenu_game;
    if (ret == 1 || ret == 3)
        return qeMenu_settings;
    if (ret == 2)
        return qeMenu_info;
    if (ret == 4)
        return gameAction_addUser;

    return ret;
}

//----------------------------------------------------------------------------------------------------

int mainmenu_info(GameRenderer *gr) {
    Menu menu;
    string opt[] = {

    };
    int n = 0;
    string qeMenu[] = {"Game", "Settings", "Info"};
    int qeN = 3;
    string title = "StarCamp";

    Text txt1, txt2, txt3, txt4;
    txt1.changeText(gr->cam.r, "Author: Tilen Stermecki, single-dev game", 30);
    txt1.move(70, 3 * 50 * 1.3);
    txt2.changeText(gr->cam.r, "School: Vegova Ljubljana, class R 4. B", 20);
    txt2.move(70, 3.6 * 50 * 1.3);
    txt3.changeText(gr->cam.r, "Graduation thesis for school year 2023/24", 20);
    txt3.move(70, 4.3 * 50 * 1.3);
    txt4.changeText(gr->cam.r, "In development since August 2023", 20);
    txt4.move(70, 4.8 * 50 * 1.3);
    txt4.changeText(gr->cam.r, "Written entirely in C/C++, using only esential libraries", 20);
    txt4.move(70, 5.2 * 50 * 1.3);

    int ret = menu.chose(gr->cam.r, opt, n, title, qeMenu, qeN, 2, // static options
                         &txt1, &txt2, &txt3, &txt4);              // added features

    txt1.destroy();
    txt2.destroy();
    txt3.destroy();
    txt4.destroy();

    if (ret == 0)
        return qeMenu_game;
    if (ret == 1)
        return qeMenu_settings;
    if (ret == 2)
        return qeMenu_info;

    return ret;
}