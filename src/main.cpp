#include <iostream>
#include <SDL2/SDL.h>
#include "mouse/mouse.h"
#include "keyboard/keyboard.h"
#include "phisics/phisics.h"
#include "timer/timer.h"

using namespace std;

#define WIDTH 640
#define HEIGHT 480

#define PHISICS_SUBSTEPS 20

int main(int argc, char *argv[]) {
    Camera cam;
    debCam = &cam;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Window *wind = SDL_CreateWindow("But we have AI at home...", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0); // SDL_WINDOW_BORDERLESS namesto 0
    if (!wind) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    cam.assignRenderer(SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!cam.r) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return 0;
    }

    SDL_SetRenderDrawBlendMode(cam.r, SDL_BLENDMODE_BLEND);

    TTF_Init();
    TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Regular.ttf", 24);
    // SDL_Surface *textSurface;
    // textSurface = TTF_RenderText_Blended(Sans, "Hello world!", SDL_Color({255, 255, 255})); //use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    // SDL_Texture *textTexture = SDL_CreateTextureFromSurface(r, textSurface);

    bool running = true;
    Mouse m;
    Keyboard kb;
    SDL_Event event;
    Timer t;

    PhWorld phisics;
    phisics.createNewLineObst(-.1, -7, 6.1, -7);
    phisics.createNewLineObst(0, -.9, 0, -7.1);
    phisics.createNewLineObst(6, -7.1, 6, -.9);
    phisics.createNewLineObst(6.1, -1, -.1, -1);

    phisics.createNewPoint(2, -5, 1);
    phisics.createNewPoint(4, -5, 1);
    phisics.createNewPoint(3, -3, 1);

    phisics.createNewLinkBetween(0, 1);
    phisics.createNewLinkBetween(2, 0);
    phisics.createNewMuscleBetween(1, 2);

    // phisics.points.at(0)->collisionGroups.pop_back();

    // phisics.createNewPoint(2, -5, 1);
    // phisics.createNewPoint(5, -5, 1);
    // phisics.createNewPoint(5, -2, 1);
    // phisics.createNewPoint(2, -2, 1);
    // phisics.createNewPoint(3.5, -3.5, 1);

    // phisics.createNewLinkBetween(0, 1);
    // phisics.createNewLinkBetween(1, 2);
    // phisics.createNewLinkBetween(2, 3);
    // phisics.createNewLinkBetween(3, 0);
    //
    // phisics.createNewLinkBetween(0, 4);
    // phisics.createNewLinkBetween(1, 4);
    // phisics.createNewLinkBetween(2, 4);
    // phisics.createNewLinkBetween(3, 4);

    cam.x = -2;
    cam.scale = 50;
    cam.y = -cam.h / cam.scale;

    while (running) {
        double dt = t.interval();

        while (SDL_PollEvent(&event)) {
            kb.update(event);
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;

                case SDL_SCANCODE_SPACE:
                    phisics.points.at(0)->move(0, -2);
                    phisics.points.at(0)->accel = {1, 5};
                    break;

                default:
                    break;
                }

                break;

            case SDL_KEYUP:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_SPACE:
                    break;

                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        uint8_t m_ev = m.update();
        if (m_ev & Mouse::M_LClickMask) {
            cout << "L click at " << m.x << ", " << m.y << endl;
        }
        if (m_ev & Mouse::M_RClickMask) {
            cout << "R click at " << m.x << ", " << m.y << endl;
        }

        if (kb.get(SDL_SCANCODE_UP)) {
            phisics.muscles.at(0)->expand();
        }
        if (kb.get(SDL_SCANCODE_DOWN)) {
            phisics.muscles.at(0)->contract();
        }
        if (kb.get(SDL_SCANCODE_LEFT)) {
            phisics.muscles.at(0)->relax();
        }

        double dtPerStep = dt / PHISICS_SUBSTEPS;
        for (int i = 0; i < PHISICS_SUBSTEPS; ++i) {
            if (kb.get(SDL_SCANCODE_W)) {
                for (int i = 0; i < phisics.points.size; ++i) {
                    phisics.points.at(i)->force.y += 15;
                }
            }
            if (kb.get(SDL_SCANCODE_S)) {
                for (int i = 0; i < phisics.points.size; ++i) {
                    phisics.points.at(i)->force.y -= 15;
                }
            }
            if (kb.get(SDL_SCANCODE_A)) {
                for (int i = 0; i < phisics.points.size; ++i) {
                    phisics.points.at(i)->force.x -= 15;
                }
            }
            if (kb.get(SDL_SCANCODE_D)) {
                for (int i = 0; i < phisics.points.size; ++i) {
                    phisics.points.at(i)->force.x += 15;
                }
            }

            phisics.applyGravity();
            phisics.update(dtPerStep);
        }

        SDL_SetRenderDrawColor(cam.r, 5, 5, 5, 255); // r b g a
        SDL_RenderClear(cam.r);

        phisics.render(&cam);

        for (uint16_t y = 0, y2 = 0; y < cam.h; y += cam.scale) {
            if (y2) {
                SDL_SetRenderDrawColor(cam.r, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(cam.r, 0, 0, 255, 255);
            }
            y2 = !y2;
            SDL_RenderDrawLine(cam.r, 0, y, 0, y + cam.scale);
        }

        SDL_RenderPresent(cam.r);
    }
    TTF_Quit();
    SDL_DestroyRenderer(cam.r);
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
}