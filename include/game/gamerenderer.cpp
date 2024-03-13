#include "game/game.h"

void GameRenderer::init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return;
    }
    wind = SDL_CreateWindow("StarCamp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0); // SDL_WINDOW_BORDERLESS namesto 0
    if (!wind)
    {
        cout << "Error creating window: " << SDL_GetError() << endl;
        SDL_Quit();
        return;
    }
    cam.assignRenderer(SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE));
    if (!cam.r)
    {
        cout << "Error creating renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return;
    }

    SDL_SetRenderDrawBlendMode(cam.r, SDL_BLENDMODE_BLEND);
    cout << "- " << SDL_GetError() << endl;

    TTF_Init();
    // TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Regular.ttf", 24);
    // SDL_Surface *textSurface;
    // textSurface = TTF_RenderText_Blended(Sans, "Hello world!", SDL_Color({255, 255, 255})); //use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    // SDL_Texture *textTexture = SDL_CreateTextureFromSurface(r, textSurface);
}

void GameRenderer::destroy()
{
    TTF_Quit();
    SDL_DestroyRenderer(cam.r);
    SDL_DestroyWindow(wind);
    SDL_Quit();
}

void GameRenderer::clear()
{
    SDL_SetRenderDrawColor(cam.r, 5, 5, 5, 255); // r b g a
    SDL_RenderClear(cam.r);
}
void GameRenderer::represent()
{
    SDL_RenderPresent(cam.r);
}
/// @brief basic SDL_Event handeling (quitting (X, ESC),...)
/// @return true on exit of a program
bool GameRenderer::basicEvents()
{
    SDL_Event event;
    bool ret = false;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            ret = true;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_ESCAPE:
                ret = true;
                break;

            default:
                break;
            }

            break;
        default:
            break;
        }
    }
    return ret;
}