#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "FastCont/FastCont.h"
#include "phisics/phisics.h"

using namespace std;

class Planet {
    int w, h;
    Point3 pos;
    double sizeMult;

public:
    SDL_Texture *texture;

    Planet();
    ~Planet();

    void generate(Camera *, int, int, int, Point3, int, int, int, bool);
    void render(Camera *);
};

#include "graphics/graphics.cpp"
#include "graphics/planet.cpp"