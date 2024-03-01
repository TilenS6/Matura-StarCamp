#pragma once

#include <iostream>
#include "camera/camera.h"
#include "mouse/mouse.h"
#include "shapes/shapes.h"
#include "FastCont/FastCont.h"

class Button
{
    SDL_Rect rect = {0, 0, 0, 0};
    string prompt = "";
    SDL_Texture *textTexture;

public:
    void create(string, Camera *, int, int, int, int);
    bool update(Mouse *);
    void render(Camera *);
};

class Menu
{
    FastCont<Button> buttons;

public:
    void create();
};
#include "mainmenu/button.cpp"