#include "menu/menu.h"

Text::Text() {
    text = NULL;
    strLink = NULL;
    current = "";
    textRect = {0, 0, 0, 0};
    t.interval();
}
void Text::destroy() {
    if (text != NULL) SDL_DestroyTexture(text);
}

void Text::linkText(string *str) {
    strLink = str;
    *strLink = current;
}
void Text::changeText(SDL_Renderer *r, string a, int size) {
    if (r == NULL) r = last_r;
    if (a == "") a = last_a;
    if (size == 0) size = last_size;
    if (r == NULL || a == "" || size == 0) return;
    last_r = r;
    last_a = a;
    last_size = size;

    current = a;

    if (text != NULL) SDL_DestroyTexture(text);

    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", size);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, a.c_str(), SDL_Color({TEXT_COLOUR})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    text = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);

    TTF_CloseFont(font);
}
void Text::move(int x, int y) {
    textRect.x = x;
    textRect.y = y;
}
void Text::render(SDL_Renderer *r) {
    double sec = t.getTime() * 1.4 - (textRect.y / 500.);
    if (sec < 0) sec = 0;
    if (sec > 1) sec = 1;
    int alpha = sec * 255;
    SDL_SetTextureAlphaMod(text, alpha);

    if (strLink != NULL) {
        string tmp = *strLink;
        if (tmp != current) {
            changeText(NULL, tmp, 0);
        }
    }

    SDL_RenderCopy(r, text, NULL, &textRect);
}