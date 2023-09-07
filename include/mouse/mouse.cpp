#include "mouse/mouse.h"

uint8_t Mouse::update() {
    uint8_t ret = 0;

    buttons = SDL_GetMouseState(&x, &y);
    left = buttons & SDL_BUTTON_LMASK;
    right = buttons & SDL_BUTTON_RMASK;
    if (buttons != lastButtons) ret |= M_ButtonMask;
    if (lastX != x || lastY != y) ret |= M_MovementMask;
    if (!(lastButtons & SDL_BUTTON_LMASK) && left) ret |= M_LClickMask;
    if (!(lastButtons & SDL_BUTTON_RMASK) && right) ret |= M_RClickMask;

    lastX = x;
    lastY = y;
    lastButtons = buttons;

    return ret;
}
