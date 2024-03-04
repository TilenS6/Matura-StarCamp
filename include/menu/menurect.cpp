#include "menu/menu.h"

MenuRect::MenuRect() {
    t.interval();
}

void MenuRect::render(SDL_Renderer *r) {
    double sec = t.getTime() * 1.4 - (y / 500.);
    if (sec < 0) sec = 0;
    if (sec > 1) sec = 1;
    int rendW = w * sin(sec * PIh);
    int rendH = h * sin(sec * PIh);
    int alpha = sec * 255;

    SDL_SetRenderDrawColor(r, MENU_COLOUR2, alpha); // r b g a
    SDL_Rect rect = {x, y, rendW, rendH};
    SDL_RenderDrawRect(r, &rect);
}