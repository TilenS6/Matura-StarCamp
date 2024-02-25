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
#include <thread>
#include "graphics/graphics.h"

#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5
#define NETW_REQ_INTERVAL 0.1
#define MAX_DT 0.005
#define CAMERA_STIFFNESS .3 // s kksno vrednostjo ostane stara vrednost pozicije kamere po 1s

using namespace std;
// class Player;
class Particle;
class ParticleS;
class Generator;
class Game;

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

#define readBuff(buff, offset, a)         \
    memcpy(&a, buff + offset, sizeof(a)); \
    offset += sizeof(a);
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
    Game *g = nullptr;

public:
    void init(Game *);
    void newPlayerAt(Point, int);

    unsigned long PlanetGenSeed;
    int PlanetCount;
    void planets(unsigned long, int);
};

class Game {
    Camera cam;
    SDL_Window *wind;

    Mouse m;
    Keyboard kb;
    Timer t;

    PhWorld phisics;

    Generator gen;

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

    // ---- gameplay ----
    Circle gameArea;

public:
    Game();
    ~Game();
    void update();
    void render();

    bool looping() { return running; }
    static void networkManagerC(Game *); // static zarad thread-ov
    static void networkManagerS(Game *); // static zarad thread-ov

    void requestInitialFromServer();
    void requestUpdateAllFromServer();
    void process_init();
    void process_update_all();
    void send_init(int, int);
    void send_update_all(int);

    void send_updatePlayerControls();
    void process_updatePlayerControls(RecievedData *);

    void send_bye();
    void send_removedPoints(int); // removed points logged in removedPoints
    void process_deletePoints();

    void handle_newPlayer(int);
    void followCamera(double);

    void handle_playerLeft(int);

    friend class Generator;
};

#include "game/generator.cpp"
#include "game/game.cpp"

// TODO: teksture gor na clientih + player.h dej nekak stran, nared "generator.h" al neki