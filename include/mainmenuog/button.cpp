#include "mainmenuog/mainmenuog.h"

Button::Button() {
    text = NULL;
    lastMLeft = true;
}

void Button::move(int x, int y, int w, int h) {
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    textRect.x = _x + (_x - textRect.w) / 2;
    textRect.y = _y + (_y - textRect.h) / 2;
}

void Button::changeText(SDL_Renderer *r, string a) {
    if (text != NULL) SDL_DestroyTexture(text);

    TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Bold.ttf", 24);
    SDL_Surface *textSurface = TTF_RenderText_Blended(Sans, a.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    text = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);

    textRect.x = _x + (_w - textRect.w) / 2;
    textRect.y = _y + (_h - textRect.h) / 2;

    TTF_CloseFont(Sans);
}

bool Button::clicked(Mouse m) {
    bool res = !lastMLeft && m.left && m.x >= _x && m.x <= _x + _w && m.y >= _y && m.y <= _y + _h;
        
    lastMLeft = m.left;
    return res;
}

void Button::render(SDL_Renderer *r) {
    SDL_Rect borders = {_x, _y, _w, _h};

    SDL_RenderCopy(r, text, NULL, &textRect);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &borders);
}