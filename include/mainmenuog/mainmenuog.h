#pragma once
#include "camera/camera.h"
#include "mouse/mouse.h"
#include "shapes/shapes.h"
#include "FastCont/FastCont.h"

class Button {
    int _x, _y, _w, _h;
    bool lastMLeft;
    SDL_Texture *text;
    SDL_Rect textRect;

public:
    Button();
    void move(int, int, int, int);
    void changeText(SDL_Renderer *, string);
    bool clicked(Mouse);
    void render(SDL_Renderer *);
};

class MainMenu {
    FastCont<Button> btns;

public:
    int chose(SDL_Renderer *, string[], uint8_t, string = "");
};

#include "mainmenuog/button.cpp"
#include "mainmenuog/mainmenu.cpp"