#include "menu/menu.h"

Cursor::Cursor() {
    texture = NULL;
}
Cursor::~Cursor() {
    if (texture != NULL) SDL_DestroyTexture(texture);
}

void Cursor::init(Mouse *_m, SDL_Renderer *r, double _radius = 20) {
    m = _m;
    rad = _radius;

    pos.x = m->x;
    pos.y = m->y;

    SDL_Texture *org = SDL_GetRenderTarget(r);
    texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, rad * 2 + 3, rad * 2 + 3);
    SDL_SetRenderTarget(r, texture);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);

    SDL_SetRenderDrawColor(r, DETAILS_COLOUR, 255);
    SDL_Circle(r, rad, rad, rad);
    SDL_Circle(r, rad + 1, rad, rad);
    SDL_Circle(r, rad, rad + 1, rad);
    SDL_Circle(r, rad + 1, rad + 1, rad);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(r, org);
}
void Cursor::update(double dt) {
    double k = pow(CURSOR_STIFFNESS, dt);
    pos.x = pos.x * k + m->x * (1. - k);
    pos.y = pos.y * k + m->y * (1. - k);

    double dx = pos.x - m->x;
    double dy = pos.y - m->y;
    dir = atan2(dy, dx);
    spd = (dx * dx + dy * dy) * dt * .5;
    if (spd > 50) spd = 50;
}
void Cursor::render(SDL_Renderer *r, double displayRad) {
    SDL_FRect dest = {
        (float)(pos.x - displayRad),
        (float)(pos.y - displayRad),
        (float)(displayRad * 2 + 3 + spd),
        (float)(displayRad * 2 + 3 - spd / 3),
    };

    SDL_FPoint center = {(float)displayRad + 1, (float)displayRad + 1};
    SDL_RenderCopyExF(r, texture, NULL, &dest, (dir / PI) * 180, &center, SDL_FLIP_NONE);

    // SDL_SetRenderDrawColor(r, 255, 255, 255, 100);
    // SDL_Circle(r, m->x, m->y, displayRad * .8);
}