#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mouse/mouse.h"
#include "keyboard/keyboard.h"
#include "phisics/phisics.h"
#include "timer/timer.h"

#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5

using namespace std;
class Player;
class Particle;
class ParticleS;
class Game;

class Player {
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
    FastCont<int> ids;  // 8+1
    FastCont<int> thrs; // 8

    SDL_Texture *texture;

public:
    void init(PhWorld *, Keyboard *, Camera *, double, double);
    void update();
    void render(Camera *);
};

class Particle {
    Rectangle r;
    Point accel;
    double accel_mult_second; // accel *= 1 - ((1 - accel_mult_second) * dt);
    double rem_life, init_life;
    SDL_Colour colour;

public:
    void init(Point, double, Point, double, double, unsigned char, unsigned char, unsigned char);
    bool update(double); // return: true=brisi sebe
    void render(Camera *);
};

class ParticleS {
    FastCont<Particle> ps;

public:
    Point pos;
    double dir, speed, life;
    double randDir, randSpeed, randLife; // vse +/-
    ParticleS(Point, double, double, double);
    void update(double);
    void render(Camera *);
};

class Game {
    Camera cam;
    SDL_Window *wind;

    Mouse m;
    Keyboard kb;
    Timer t;

    PhWorld phisics;
    ParticleS particleSystem;

    Player player;

    bool running;
    bool drawRuller = false;

public:
    Game();
    ~Game();
    void update();

    bool looping() { return running; }
};

#include "game/game.cpp"
#include "game/player.cpp"