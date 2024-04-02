#include "graphics/graphics.h"

Star::Star() {
    texture = nullptr;
}
Star::~Star() {
    if (texture != nullptr)
        SDL_DestroyTexture(texture);
}

void Star::generate(Camera *cam, int _w, int _h, int realW, Point3 at, int rd = 255, int gr = 255, int bl = 255, double intensity = 1) { // TODO intensity unused
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    w = _w+1;
    h = _h+1;

    sizeMult = (double)realW / (double)w;

    int centerX = w / 2;
    int centerY = h / 2;
    pos = at;

    texture = SDL_CreateTexture(cam->r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_Texture *orgTarget = SDL_GetRenderTarget(cam->r);
    SDL_BlendMode orgBlend;
    SDL_GetRenderDrawBlendMode(cam->r, &orgBlend);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(cam->r, texture);
    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 0);
    SDL_RenderClear(cam->r);
    // -------- risanje

    SDL_SetRenderDrawColor(cam->r, rd, gr, bl, 255);
    SDL_FilledCircle(cam->r, centerX, centerY, centerX);
    // TODO testirej

    // -------- nazaj render target
    SDL_SetRenderTarget(cam->r, orgTarget);
    SDL_SetRenderDrawBlendMode(cam->r, orgBlend);
}
void Star::render(Camera *cam) {
    if (texture == nullptr) {
        cout << "texture not initialized @ Star::render!\n";
        return;
    }

    Point at = pos.renderAt(cam);
    double k = pow(.5, pos.z) * sizeMult;

    SDL_FRect rect;
    rect.x = at.x;
    rect.y = at.y;
    rect.w = (w * cam->scale) * k;
    rect.h = (h * cam->scale) * k;
    rect.x -= rect.w / 2.;
    rect.y -= rect.h / 2.;

    if (rect.x > cam->w) return;
    if (rect.y > cam->h) return;
    if (rect.x + rect.w < 0) return;
    if (rect.y + rect.h < 0) return;

    SDL_RenderCopyF(cam->r, texture, NULL, &rect);
}