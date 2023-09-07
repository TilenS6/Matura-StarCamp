#pragma once
#include <iostream>
#include <SDL2/SDL.h>

class Keyboard {
    const uint8_t n = 64;
    uint8_t arr[64]; // each key is single bit, marked by SDL_scancode.h

public:
    Keyboard();
    void down(SDL_Scancode);
    void up(SDL_Scancode);
    void update(SDL_Event);

    bool get(SDL_Scancode);
};

#include "keyboard/keyboard.cpp"