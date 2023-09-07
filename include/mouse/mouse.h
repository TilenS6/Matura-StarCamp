#pragma once
#include <iostream>
#include <SDL2/SDL.h>
using namespace std;

class Mouse {
    int lastX, lastY;
    Uint32 lastButtons = 0;

public:
    int x, y;
    bool left = false, right = false;
    Uint32 buttons = 0;

    enum UpdateTypeMask {
        M_ButtonMask = 1,
        M_MovementMask = 2,
        M_LClickMask = 4,
        M_RClickMask = 8,
    };

    uint8_t update();
};

#include "mouse/mouse.cpp"