#pragma once
#include <SDL2/SDL.h>
#include "mouse/mouse.h"
#include "keyboard/keyboard.h"
#include "phisics/phisics.h"
#include "timer/timer.h"

#define WIDTH 1920 / 2
#define HEIGHT 1080 / 2
#define PHISICS_SUBSTEPS 5

using namespace std;

class Game {
    Camera cam;
    SDL_Window *wind;
	
    Mouse m;
    Keyboard kb;
    Timer t;

    PhWorld phisics;
	
    bool running;
	bool drawRuller = false;
public:
    Game();
	~Game();
	void update();

	bool looping() { return running; }
};

#include "game/game.cpp"