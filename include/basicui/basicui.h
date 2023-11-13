#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include "camera/camera.h"

using namespace std;
#define MAX_FONT_SIZE 128

class BUI {
    Camera *c = nullptr;
	TTF_Font *Sans[MAX_FONT_SIZE];
public:
    BUI();
    void destroy();
    void assignCamera(Camera *);
    SDL_Rect drawText(string, int, int, int, Uint8, Uint8, Uint8);
    string stringInput(string);
};

#include "basicui/basicui.cpp"