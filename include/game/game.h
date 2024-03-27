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
#include "inventory.h"

#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5
#define NETW_REQ_INTERVAL 0.05
#define MAX_DT 0.005
#define CAMERA_STIFFNESS .3 // s kksno vrednostjo ostane stara vrednost pozicije kamere po 1s

#define ANIMATION_SPEED 0.01
#define BUTTON_DISTANCE 1.5

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

#define readBuff(buff, offset, a)         \
    memcpy(&a, buff + offset, sizeof(a)); \
    offset += sizeof(a);

#define readBuff_c(buff, offset, bufflen, a) \
    if (offset + sizeof(a) > bufflen) {      \
        requestInitialFromServer();          \
        return;                              \
    } else {                                 \
        readBuff(buff, offset, a);           \
    };

using namespace std;
// class Player;
class Particle;
class ParticleS;
class Generator;
class Game;

class Generator {
    Game *g = nullptr;

public:
    void init(Game *);
    void newPlayerAt(Point, int);

    unsigned long PlanetGenSeed;
    int PlanetCount;
    void planets(unsigned long, int);
    void stars(int);
};

class GameRenderer {
    SDL_Window *wind;

public:
    Camera cam;

    void init();
    void destroy();

    void clear();
    void represent();
    bool basicEvents();

    friend class Game;
};

// ------ INTERACTIVE --------
class InteractiveDropoffArea {
    Rectng rect;
    DroppedItem containing;

public:
    double rotation; // !! in DEG !!
    bool hijacked;
    InteractiveDropoffArea();
    void setRect(double, double, double, double);

    bool update(FastCont<DroppedItem> *, Inventory *);
    void pickupToInv(Inventory *);
    void updateHijack(Keyboard *, Mouse *, Inventory *, Camera *);

    void render(Camera *);
};

class InteractiveButton {
    SDL_Texture *textT;
    int tw, th;
    string txt;
    
    Point pos;
    double animationK;

public:
    InteractiveButton();
    void init(Point, string, Camera *);
    bool update(Point, double, Keyboard *); // TODO nej shran void* do funkcije, nej jo klice ko je aktiveran
    void render(Camera *);
};

// -------- GAME CLASS --------
struct LoginEntry {
    string username, password;
    Point logoutPos;
};

FastCont<LoginEntry> login;
class Game {
    GameRenderer *grend;
    SDL_Window *wind;

    Mouse m;
    Keyboard kb;
    Timer t;

    PhWorld phisics;

    Generator gen;

    // ---- client ----
    Inventory client_inventory;
    Point playerMedian;

    // ---- network ----
    NetClient client;
    NetServer server;
    FastCont<int> clientIds; // everyone in server [loginID] => [clientConnectionID]
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
    FastCont<DroppedItem> droppedItems;
    FastCont<InteractiveDropoffArea> dropoffAreas;
    FastCont<InteractiveButton> intButtons;

    string quitInfo;

    void renderHUD();

public:
    Game(GameRenderer *, string, string, string, bool);
    ~Game();
    void end();

    // -- basic
    void update();
    void render();

    bool looping() { return running; }

    // -- network
    static void networkManagerC(Game *); // static zarad thread-ov
    static void networkManagerS(Game *); // static zarad thread-ov

    void requestInitialFromServer();
    void send_init(int, int);
    void process_init();

    void requestUpdateAllFromServer();
    void send_update_all(int);
    void process_update_all();

    void send_updatePlayerControls();
    void process_updatePlayerControls(RecievedData *);

    void send_bye();
    void send_removedPoints(int); // removed points logged in removedPoints
    void process_deletePoints();

    void handle_newPlayer(int);
    void handle_playerLeft(int);
    void followCamera(double);

    void sendLoginInfo(string, string);
    int resolveLoginInfo(RecievedData *);

    void sendPickup(int, DroppedItem); // server use
    void process_pickup();             // client use

    void sendDrop(DroppedItem);        // client use
    void process_drop(RecievedData *); // server user

    // -- inventory (v inventory.cpp)
    int dropInventoryItem(int, int, Point);
    void updatePlayersPickupFromFloor();
    void updateInteractiveItems();

    void renderDroppedItems(Camera *);

    // getterji/setterji
    string getQuitInfo() { return quitInfo; }

    friend class Generator;
};

#include "generator.cpp"
#include "game_networking_c.cpp"
#include "game_networking_s.cpp"
#include "gamerenderer.cpp"
#include "game_inventory.cpp"
#include "interactive.cpp"
#include "game.cpp"