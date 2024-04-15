#include "mainmenuog/mainmenuog.h"

int Menu::chose(SDL_Renderer *r, string s[], uint8_t n, string title) {
    int W, H;
    SDL_GetRendererOutputSize(r, &W, &H);
    int btnW = 200, btnH = 50;

    btns.clear();

    for (uint8_t i = 0; i < n; ++i) {
        Button tmp;
        int id = btns.push_back(tmp);
        Button *p = btns.at_id(id);
        p->move((W - btnW) / 2, (btnH * (i + 1)) * 1.2, btnW, btnH); // na sredino ekrana
        p->changeText(r, s[i]);
    }

    TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-ExtraBold.ttf", 40);
    SDL_Surface *textSurface = TTF_RenderText_Blended(Sans, title.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_Rect titleTextureRect;
    SDL_QueryTexture(titleTexture, NULL, NULL, &titleTextureRect.w, &titleTextureRect.h);

    titleTextureRect.x = (W - titleTextureRect.w) / 2;
    titleTextureRect.y = 0;

    TTF_CloseFont(Sans);

    Mouse m;
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;
                default:
                    break;
                }

                break;
            }
        }
        m.update();

        uint8_t i = 0;
        for (int i = 0; i < btns.size(); ++i) {
            if (btns.at_index(i)->clicked(m)) {
                return i;
            }
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255); // r b g a
        SDL_RenderClear(r);

        SDL_RenderCopy(r, titleTexture, NULL, &titleTextureRect);

        for (int i = 0; i < btns.size(); ++i) {
            btns.at_index(i)->render(r);
        }

        SDL_RenderPresent(r);
    }
    SDL_DestroyTexture(titleTexture);
    return -1;
}