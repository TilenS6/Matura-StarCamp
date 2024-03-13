#pragma once
#define GAME_EXISTS
// #define CONSOLE_LOGGING

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mouse/mouse.h"
#include "keyboard/keyboard.h"
#include "phisics/phisics.h"
#include "timer/timer.h"
#include "particles/particles.h"
#include "netagent/netagent.h"
#include "netagent/netstds.cpp"
#include <thread>
#include "graphics/graphics.h"
#include "basicui/basicui.h"

#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5
#define NETW_REQ_INTERVAL 0.05
#define MAX_DT 0.005
#define CAMERA_STIFFNESS .3 // s kksno vrednostjo ostane stara vrednost pozicije kamere po 1s

#define INVENTORY_SIZE 5
#define INVENTORY_TEXTURE_SIZE 50
#define INVENTORY_TEXTURE_BORDER 2
#define INVENTORY_TEXTURE_BORDER_COLOUR 255, 255, 255

using namespace std;
// class Player;
class Particle;
class ParticleS;
class Generator;
class Game;

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

#define readBuff(buff, offset, a) \
    memcpy(&a, buff + offset, sizeof(a)); \
    offset += sizeof(a);

#define readBuff_c(buff, offset, bufflen, a) \
        if (offset + sizeof(a) > bufflen) { \
            requestInitialFromServer(); \
            return; \
        } else { \
            readBuff(buff, offset, a); \
        };
/*
class Player
{
    double p[8][2] = {
        // initial points
        {1, 1},
        {0, 2},
        {1, 3},
        {2, 3},
        {3, 2},
        {2, 1},
        {2, 0.3},
        {1, 0.3},
    };
    Point p_min, p_max, p_avg;

    PhWorld *w;
    Keyboard *kb;
    int centerId;
    FastCont<int> ids;    // 8+1
    FastCont<int> thrsId; // 8

    SDL_Texture *texture;

public:
    void init(PhWorld *, Keyboard *, Camera *, double, double);
    void update();
    void render(Camera *);

    friend class Game;
};

*/

class Generator {
    Game* g = nullptr;

public:
    void init(Game*);
    void newPlayerAt(Point, int);

    unsigned long PlanetGenSeed;
    int PlanetCount;
    void planets(unsigned long, int);
    void stars(int);
};

class GameRenderer {
    SDL_Window* wind;

public:
    Camera cam;

    void init();
    void destroy();

    void clear();
    void represent();
    bool basicEvents();

    friend class Game;
};

struct InventoryEntry {
    int ID, count;
};
class Inventory {
    public:
    InventoryEntry inv[INVENTORY_SIZE];
    int selected;

    Inventory();
    void render(Camera *);
};

struct LoginEntry {
    string username, password;
    Point logoutPos;
};

FastCont<LoginEntry> login;
class Game {
    GameRenderer* grend;
    SDL_Window* wind;

    Mouse m;
    Keyboard kb;
    Timer t;

    PhWorld phisics;

    Generator gen;

    // ---- client ----
    Inventory client_inventory;

    // ---- network ----
    NetClient client;
    NetServer server;
    Timer netRequestTimer;
    thread networkThr;
    bool running, networkingActive;
    bool serverRole;
    bool halt = false, halting = false;
    FastCont<double> thrSendBuffer;
    FastCont<int> removedPoints;

    // ---- visual ----
    FastCont<ParticleS> particleSs;
    bool drawRuller = false;
    FastCont<Planet> planets;
    FastCont<Star> stars;

    // ---- gameplay ----
    Circle gameArea;
    FastCont<int> clientIds; // everyone in server [loginID] => [clientConnectionID]

    void renderHUD();

public:
    Game(GameRenderer*, string, string, string, bool);
    ~Game();
    void update();
    void render();

    bool looping() { return running; }
    static void networkManagerC(Game*); // static zarad thread-ov
    static void networkManagerS(Game*); // static zarad thread-ov

    void requestInitialFromServer();
    void requestUpdateAllFromServer();
    void process_init();
    void process_update_all();
    void send_init(int, int);
    void send_update_all(int);

    void send_updatePlayerControls();
    void process_updatePlayerControls(RecievedData*);

    void send_bye();
    void send_removedPoints(int); // removed points logged in removedPoints
    void process_deletePoints();

    void handle_newPlayer(int);
    void followCamera(double);

    void handle_playerLeft(int);

    // -------- login --------
    void sendLoginInfo(string, string);
    int resolveLoginInfo(RecievedData*);

    friend class Generator;
};

#include "game/generator.cpp"
#include "game_networking_c.cpp"
#include "game_networking_s.cpp"
#include "game/gamerenderer.cpp"
#include "game/inventory.cpp"
#include "game/game.cpp"