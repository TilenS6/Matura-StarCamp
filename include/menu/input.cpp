#include "menu/menu.h"

Input::Input() {
    t.interval();
    texture = NULL;
}

void Input::destroy() {
    if (texture != NULL)
        SDL_DestroyTexture(texture);
}
string Input::getStringFromSDL(SDL_Renderer *r) {
    string out = *str;
    bool input = true;
    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
    SDL_Event event;

    string rendStr = "";

    SDL_StartTextInput();
    while (input) {
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                /* Quit */
                input = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_KP_ENTER:
                    input = false;
                    break;
                case SDL_SCANCODE_BACKSPACE:
                    if (!out.empty()) out.pop_back();
                    break;

                default:
                    break;
                }

                break;

            case SDL_TEXTINPUT:
                /* Add new text onto the end of our text */
                out += event.text.text;
                break;
            }
        }

        if (rendStr != out) {
            rendStr = out;
            SDL_DestroyTexture(texture);
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, (out).c_str(), SDL_Color({TEXT_COLOUR})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
            texture = SDL_CreateTextureFromSurface(r, textSurface);
            SDL_FreeSurface(textSurface);
            SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
        }

        // clear
        SDL_SetRenderDrawColor(r, BACKGROUND_COLOUR, 255); // r b g a
        SDL_Rect eraseRect = {x + 1, y + 1, w - 2, h - 2};
        SDL_RenderFillRect(r, &eraseRect);

        SDL_Rect dest = {x + 10, y + (h - textH) / 2, textW, textH};
        SDL_RenderCopy(r, texture, NULL, &dest);

        SDL_RenderPresent(r);
    }
    SDL_StopTextInput();
    TTF_CloseFont(font);
    return out;
}

void Input::linkText(SDL_Renderer *r, string *a, bool sensitiveText = false) {
    sensitive = sensitiveText;
    if (texture != NULL) SDL_DestroyTexture(texture);
    str = a;
    string displ = *str;
    if (sensitive) {
        string tmp = "";
        for (int i = 0; i < displ.length(); ++i)
            tmp += '*';

        displ = tmp;
    }
    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, displ.c_str(), SDL_Color({TEXT_COLOUR})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    texture = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);

    SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
}
void Input::update(Mouse *m, SDL_Renderer *r) {
    bool res = !lastMLeft && m->left && m->x >= x && m->x <= x + w && m->y >= y && m->y <= y + h;
    lastMLeft = m->left;

    if (res) {
        *str = getStringFromSDL(r);

        if (sensitive) {
            string displ = *str;
            string tmp = "";
            for (int i = 0; i < displ.length(); ++i)
                tmp += '*';

            TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, tmp.c_str(), SDL_Color({TEXT_COLOUR})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
            texture = SDL_CreateTextureFromSurface(r, textSurface);
            SDL_FreeSurface(textSurface);
            TTF_CloseFont(font);

            SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
        }
    }
}
void Input::render(SDL_Renderer *r) {
    double sec = t.getTime() * 1.4 - (y / 500.);
    if (sec < 0) sec = 0;
    if (sec > 1) sec = 1;
    int rendW = w * sin(sec * PIh);
    int rendH = h * sin(sec * PIh);
    int alpha = sec * 255;
    SDL_SetTextureAlphaMod(texture, alpha);

    SDL_SetRenderDrawColor(r, MENU_COLOUR2, alpha); // r b g a
    SDL_Rect rect = {x, y, rendW, rendH};
    SDL_RenderDrawRect(r, &rect);

    if (texture != NULL) {
        SDL_Rect dest = {x + 10, y + (h - textH) / 2, textW, textH};
        SDL_RenderCopy(r, texture, NULL, &dest);
    }
}