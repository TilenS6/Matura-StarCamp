#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "mouse/mouse.h"
#include "keyboard/keyboard.h"
#include "phisics/phisics.h"
#include "timer/timer.h"

using namespace std;

class Particle;
class ParticleS;

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
    Particle init;
    double spwnTimer;

public:
    double randDir, randSpeed, randLife; // vse +/-
    bool spawning;
    
    void setInitial(Particle p); // TODO
    // TODO void setRandomises(...)
    // TODO void setSpawning(...)
    void create(Point, double, Point, double, double, unsigned char, unsigned char, unsigned char, double);
    void update(double);
    void render(Camera *);
};

#include "particles/particles.cpp"