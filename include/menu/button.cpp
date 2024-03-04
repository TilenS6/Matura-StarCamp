#include "menu/menu.h"

Button::Button() {
    text = NULL;
    lastMLeft = true;
    t.interval();
    style = 0;
    highlighted = false;
    animated = true;
}

void Button::destroy() {
    if (text != NULL) SDL_DestroyTexture(text);
}

void Button::move(int x, int y, int w, int h) {
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    textRect.x = _x;
    textRect.y = _y;
}

void Button::changeText(SDL_Renderer *r, string a) {
    if (text != NULL) SDL_DestroyTexture(text);

    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, a.c_str(), SDL_Color({TEXT_COLOUR})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    text = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);

    textRect.x = _x + (_w - textRect.w) / 2;
    textRect.y = _y + (_h - textRect.h) / 2;

    TTF_CloseFont(font);
}

void Button::changeStyle(int styleNo) {
    if (style >= 0 && style <= 1)
        style = styleNo;
}

bool Button::clicked(Mouse *m) {
    bool res = !lastMLeft && m->left && m->x >= _x && m->x <= _x + _w && m->y >= _y && m->y <= _y + _h;

    lastMLeft = m->left;
    return res;
}

void Button::render(SDL_Renderer *r) {
    if (style == 0)
        renderStyle0(r);
    else if (style == 1)
        renderStyle1(r);
}

void Button::renderStyle0(SDL_Renderer *r) {
    double sec;
    if (animated)
        sec = t.getTime() * 1.4 - (_y / 500.);
    else
        sec = 1;
    if (sec < 0) sec = 0;
    if (sec > 1) sec = 1;
    int rendW = _w * sin(sec * PIh);
    int alpha = sec * 255;
    SDL_SetTextureAlphaMod(text, alpha);

    SDL_Rect borders = {_x, _y, rendW, _h};

    int x = _x, y = _y, w = rendW, h = _h;
    SDL_Point corners[5] = {
        {x, y + h},
        {(x + (h / 2)), y},
        {x + w, y},
        {(x + w - (h / 2)), y + h},
        {x, y + h},
    };

    x += 6;
    y += 3;
    w -= 12;
    h -= 6;
    SDL_Point closer[5]{
        {x, y + h},
        {(x + (h / 2)), y},
        {x + w, y},
        {(x + w - (h / 2)), y + h},
        {x, y + h},
    };

    x -= 6 * 2;
    y -= 3 * 2;
    w += 12 * 2;
    h += 6 * 2;

    int ovW = 40, ovH = 20;
    x += (h - ovH) / 2;
    // top-left
    SDL_Point det1[3] = {
        {x, y + ovH},
        {(x + (ovH / 2)), y},
        {x + ovW, y},
    };
    x -= (h - ovH) / 2;

    x = x + w - ovW;
    y = y + h - ovH;

    x -= (h - ovH) / 2;
    // bottom-right
    SDL_Point det2[3] = {
        {x + ovW, y},
        {(x + ovW - (ovH / 2)), y + ovH},
        {x, y + ovH},
    };

    SDL_Vertex vert[4];
    vert[0] = {
        {(float)corners[0].x, (float)corners[0].y}, // position on screen
        {MENU_COLOUR2, 255},                        // colour
        {(float)0, (float)0},                       // texture normals
    };
    vert[1] = {
        {(float)corners[1].x, (float)corners[1].y}, // position on screen
        {MENU_COLOUR2, 255},                        // colour
        {(float)0, (float)0},                       // texture normals
    };
    vert[2] = {
        {(float)corners[2].x, (float)corners[2].y}, // position on screen
        {MENU_COLOUR2, 255},                        // colour
        {(float)0, (float)0},                       // texture normals
    };
    vert[3] = {
        {(float)corners[3].x, (float)corners[3].y}, // position on screen
        {MENU_COLOUR2, 255},                        // colour
        {(float)0, (float)0},                       // texture normals
    };
    int ind[6] = {
        0, 1, 2,
        0, 2, 3};

    // SDL_SetRenderDrawColor(r, MENU_COLOUR1, 255);
    // SDL_RenderFillRect(r, &borders);
    // SDL_RenderGeometry(r, NULL, vert, 4, ind, 6);

    SDL_SetRenderDrawColor(r, MENU_COLOUR1, alpha);
    // SDL_RenderDrawRect(r, &borders);
    SDL_RenderDrawLines(r, corners, 5);

    SDL_SetRenderDrawColor(r, MENU_COLOUR2, alpha);
    SDL_RenderDrawLines(r, closer, 5);

    SDL_SetRenderDrawColor(r, DETAILS_COLOUR, alpha);
    SDL_RenderDrawLines(r, det1, 3);
    SDL_RenderDrawLines(r, det2, 3);

    SDL_RenderCopy(r, text, NULL, &textRect);
}

void Button::renderStyle1(SDL_Renderer *r) {
    int ofsY = -3;
    double sec;
    if (animated)
        sec = t.getTime() * 1.4 - (_y / 500.);
    else
        sec = 1;

    if (sec < 0) sec = 0;
    if (sec > 1) sec = 1;
    int rendW = textRect.w * sin(sec * PIh);
    int alpha = sec * 255;
    SDL_SetTextureAlphaMod(text, alpha);

    // SDL_RenderDrawRect(r, &borders);
    SDL_SetRenderDrawColor(r, MENU_COLOUR2, alpha);
    SDL_RenderDrawLine(r, textRect.x, textRect.y + textRect.h + ofsY, textRect.x + rendW, textRect.y + textRect.h + ofsY);
    if (highlighted) {
        SDL_SetRenderDrawColor(r, MENU_COLOUR1, alpha);
        SDL_RenderDrawLine(r, textRect.x, textRect.y + textRect.h + 3 + ofsY, textRect.x + rendW, textRect.y + textRect.h + 3 + ofsY);
    }

    SDL_RenderCopy(r, text, NULL, &textRect);
}