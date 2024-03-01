#include "mainmenu/mainmenu.h"

void Button::create(string _prompt, Camera *cam, int x, int y, int w, int h)
{
    prompt = _prompt;
    rect = {x, y, w, h};

    TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Regular.ttf", 24);
    SDL_Surface *textSurface = TTF_RenderText_Blended(Sans, prompt.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    textTexture = SDL_CreateTextureFromSurface(cam->r, textSurface);
}
bool Button::update(Mouse *m)
{
    if (!(m->last_event & Mouse::M_LClickMask))
        return false; // uporabnik ni prtisnu miske
        
    Point mouse = {(double)m->x, (double)m->y};
    class Rectangle _rect;
    _rect.a.x = rect.x;
    _rect.a.y = rect.y;
    _rect.dimensions.x = rect.w;
    _rect.dimensions.y = rect.h;

    return collisionPointRectangle(mouse, _rect); // ce je prtisnu na gumb
}
void Button::render(Camera *cam)
{
    SDL_Rect textRect = rect;
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = rect.x + (rect.w - textRect.w) / 2;
    textRect.y = rect.y + (rect.h - textRect.h) / 2;
    
    SDL_RenderCopy(cam->r, textTexture, NULL, &textRect);
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    SDL_RenderDrawRect(cam->r, &rect);
}