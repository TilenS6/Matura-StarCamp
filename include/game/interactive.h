#pragma once
#include "game.h"

#define ANIMATION_SPEED 0.01
#define BUTTON_DISTANCE 1.5

class InteractiveDropoffArea {
    Rectng rect;
    DroppedItem containing;
    SDL_Texture *keybindTex;
    int tw, th;

public:
    double rotation; // !! in DEG !!
    bool hijacked, keybindCapturingActive;
    char thrusterKeybind;
    InteractiveDropoffArea();
    void setRect(double, double, double, double);

    bool update(FastCont<DroppedItem> *, Inventory *);
    void pickupToInv(Inventory *);
    void updateHijack(Keyboard *, Mouse *, Inventory *, Camera *);

    DroppedItem getContaining() { return containing; }

    void render(Camera *);
};

class InteractiveButton {
    SDL_Texture *textT;
    int tw, th;
    string txt;

    Point pos;
    double animationK;
    // void (*fp)(void);
    function<void()> fp;

public:
    InteractiveButton();
    void init(Point, string, Camera *, function<void()>); // void (*fPointer)()
    bool update(Point, double, Keyboard *); // TODO nej shran void* do funkcije, nej jo klice ko je aktiveran
    void render(Camera *);
};

#include "interactiveDropoffArea.cpp"
#include "interactiveButton.cpp"