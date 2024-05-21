#pragma once
#define GAME_EXISTS
// #define CONSOLE_LOGGING
// #define CONSOLE_LOGGING_INIT
// #define CONSOLE_LOGGING_PICKUP
// #define CONSOLE_LOGGING_NETWORKING
// #define CONSOLE_LOGGING_STAGES
// #define CONSOLE_LOGGING_DT_CAPPED
// #define CONSOLE_LOGGING_ID_NOT_FOUND

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
#include "interactive.h"
#include "shipbuilder.h"

// za general zadeve
#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5
#define NETW_REQ_INTERVAL 0.05
#define MAX_DT 0.005
#define CAMERA_STIFFNESS .1 // s kksno vrednostjo ostane stara vrednost pozicije kamere po 1s

// za building area
#define BUILDING_BLOCK_SIZE 1.
#define BUILDING_BLOCK_SPRING 500
#define BUILDING_BLOCK_DAMP 50
#define BUILDING_BLOCK_MERGEDISTANCE .01
#define BUILDING_FUEL_CAPACITY 20
#define BUILDING_FUEL_RECHARGE 1

// za generator asteroidov
#define GENERATION_ASTEROID_POINT_C 4
#define GENERATION_ASTEROID_POINT_C_RND 0 // +- value
#define GENERATION_ASTEROID_POINT_MASS 1
#define GENERATION_ASTEROID_POINT_NOISE 1 // +-
#define GENERATION_ASTEROID_SIZE 4
#define GENERATION_ASTEROID_SIZE_RND 1
#define GENERATION_ASTEROID_SPRING_KOEF 100
#define GENERATION_ASTEROID_DAMP_KOEF 1

#define switchValues(a, b) \
    { decltype(a) tmp = a;     \
    a = b;                 \
    b = tmp; }

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

#define readBuff(buff, offset, a)         \
    memcpy(&a, buff + offset, sizeof(a)); \
    offset += sizeof(a);

#define readBuff_c(buff, offset, bufflen, a) \
    if (offset + sizeof(a) > bufflen) {      \
        request_initialFromServer();         \
        return;                              \
    } else {                                 \
        readBuff(buff, offset, a);           \
    };

#define generateNewNoise(point, value)               \
    point.x = ((rand() % 201) - 100) * 0.01 * value; \
    point.y = ((rand() % 201) - 100) * 0.01 * value;

using namespace std;
// class Player;
class Particle;
class ParticleS;
class Generator;
class ShipBuilder;
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
    void asteroids(int, double);
    void asteroids2(int, double, Point);
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

class ShipBuilder {
    InteractiveDropoffArea areaGrid[SHIPBUILDER_GRID_H][SHIPBUILDER_GRID_W];
    Game *g;

public:
    void init(double, double, Game *);
    void build();
    void update(FastCont<DroppedItem> *, Inventory *);
    void updateHijack(Keyboard *, Mouse *, Inventory *, Camera *);
    void render(Camera *);
};

struct BuildingBlockData {
    int ID;
    char keybind;
    int rotDeg;
};

class PlayerSeat { // button on PhPoint
    InteractiveButton btn;
    Point lastPos;
    int PID = -1;
    Game *g;

    int sittingClientID; // server use

public:
    void init(int, Game *);
    void standUpCurrentPlayer();
    bool update(double); // true: pressed, send_sitdown(..)
    void render();

    friend class Game;
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
    ShipBuilder shipbuilder;

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
    FastCont<PlayerSeat> seats;

    bool sitted;

    string quitInfo;

    void renderHUD();
    int buildingShipID = 0;
    int process_buildShip_placeBlock(int, double, double, int, double, int, int, char);
    void delete_player(int);

public:
    Game(GameRenderer *, string, string, string, bool);
    ~Game();
    void end();

    // -- basic
    void update();
    void render();

    bool looping() { return running; }

    void followCamera(double);

    // -- network
    static void networkManagerC(Game *); // static zarad thread-ov
    static void networkManagerS(Game *); // static zarad thread-ov

    void request_initialFromServer();
    void send_init(int, int);
    void process_init();

    void request_updateAllFromServer();
    void send_update_all(int);
    void process_update_all();

    void send_updatePlayerControls();
    void process_updatePlayerControls(RecievedData *);

    void send_bye();
    void send_removedPoints(int); // removed points logged in removedPoints
    void process_deletePoints();

    void handle_newPlayer(int);
    void handle_playerLeft(int);

    void send_loginInfo(string, string);
    int resolve_loginInfo(RecievedData *);

    void send_pickup(int, DroppedItem); // server use
    void process_pickup();              // client use

    void send_drop(DroppedItem);       // client use
    void process_drop(RecievedData *); // server user

    void send_buildShip(BuildingBlockData *, double, double, int, int); // array, offX, offY, w, h
    void process_buildShip(RecievedData *, int);

    void send_sitdown(int);
    void process_sitdown(RecievedData *, int);
    void send_standup();
    void process_standup(RecievedData *, int);

    // -- inventory (v inventory.cpp)
    int dropInventoryItem(int, int, Point);
    void updatePlayersPickupFromFloor();
    void updateInteractiveItems();

    void renderDroppedItems(Camera *);

    // getterji/setterji
    string getQuitInfo() { return quitInfo; }

    friend class Generator;
    friend class ShipBuilder;
    friend class PlayerSeat;
};

#include "generator.cpp"
#include "shipbuilder.cpp"
#include "game_networking_c.cpp"
#include "game_networking_s.cpp"
#include "gamerenderer.cpp"
#include "game_inventory.cpp"
#include "game_shipbuilder.cpp"
#include "game.cpp"
#include "interactiveDropoffArea.cpp"
#include "interactiveButton.cpp"
#include "playerseat.cpp"

// TODO errors 4p testing:
/*
vsem neki ne dela
unknown header data
ni tekstur
skos updejta neki => prevec ful steka
*/