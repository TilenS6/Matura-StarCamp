#include "game.h"
#pragma once

#define ANIMATION_SPEED 0.01
#define BUTTON_DISTANCE 1.5

enum OnPress {
    onpress_notpressed,
    onpress_build,
    onpress_sit,
    onpress_demo,
};

class InteractiveDropoffArea {
protected:
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

    friend class ShipBuilder;
};

class InteractiveButton {
    SDL_Texture *textT;
    int tw, th;
    string txt;

    Point pos;
    double animationK;
    // void (*fp)(void);
    // function<void()> fp;
    // ShipBuilder *sb;
    int onPress;

public:
    int moreData;
    InteractiveButton();
    void init(Point, string, Camera *, int); // ShipBuilder *); //function<void()> // void (*fPointer)()
    int update(Point, double, Keyboard *);
    void render(Camera *);

    friend class PlayerSeat;
};


class OreProcessor : public InteractiveDropoffArea {
    Timer tim;
    double _processDuration;

public:
    OreProcessor();
    void setDuration(double);
    double getDuration();
    void update(FastCont<DroppedItem> *, Inventory *);
    void render(Camera *);
};

// #include "interactiveDropoffArea.cpp"
// #include "interactiveButton.cpp"