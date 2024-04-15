#include "graphics/graphics.h"

Planet::Planet() {
    texture = nullptr;
}

Planet::~Planet() {
    if (texture != nullptr)
        SDL_DestroyTexture(texture);
}
/*
void save_texture(const char *file_name, SDL_Renderer *renderer, SDL_Texture *texture) {
    SDL_Texture *target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    IMG_SavePNG(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}

bool first = true;
*/

void Planet::generate(Camera *cam, int _w, int _h, int realW, Point3 at, int rd = 255, int gr = 170, int bl = 79, bool ring = true) {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    int plW = _w, plH = _h;
    sizeMult = (double)realW / (double)plW;

    w = _w * (1 + ring * 1.5); // 2.5x sirse ce ma ring
    h = _h;
    int centerX = w / 2;
    int centerY = h / 2;

    pos = at;

    texture = SDL_CreateTexture(cam->r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w + 1, h);
    SDL_Texture *orgTarget = SDL_GetRenderTarget(cam->r);
    SDL_BlendMode orgBlend;
    SDL_GetRenderDrawBlendMode(cam->r, &orgBlend);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(cam->r, texture);
    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 0);
    SDL_RenderClear(cam->r);
    // -------- risanje

    // ring, zadnji del
    SDL_SetRenderDrawBlendMode(cam->r, SDL_BLENDMODE_BLEND);

    int holeInRing = -1, holeInRing_size;
    int ringW, ringDist;
    FastCont<int> clr;
    if (ring) {
        if ((rand() % 2) == 0)
            holeInRing = rand() % 20;

        holeInRing_size = 2 + rand() % 10;
        ringW = 10 + rand() % 21;
        ringDist = rand() % 21;
        clr.reserve_n_spots(ringW);

        int color = 150;
        for (int i = 0; i < ringW; ++i) {
            if (i == holeInRing) {
                i += holeInRing_size;
                color += rand() % 51 - 25;
                continue;
            }
            color += rand() % 11 - 5;
            color = color % 255;
            if (rand() % 5 == 0)
                color += rand() % 51 - 25;

            clr.push_back(color);
            SDL_SetRenderDrawColor(cam->r, color, color, color, 255);
            SDL_Ellipse(cam->r, centerX, centerY, w / 2 - (ringDist + i), h / 4 - ((ringDist + i) / 2), 1, 1, 0, 0);
        }
    }
//    if (first)
//        save_texture("pl_1.png", cam->r, texture);

    // planetek
    SDL_Texture *planet = SDL_CreateTexture(cam->r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w + 1, h);
    SDL_SetRenderTarget(cam->r, planet);
    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 0);
    SDL_RenderClear(cam->r);

    SDL_SetRenderDrawColor(cam->r, rd, gr, bl, 255);
    SDL_FilledCircle(cam->r, centerX, centerY, centerY);

//    if (first)
//        save_texture("pl_2.png", cam->r, planet);
    // okrogle crte
    SDL_SetRenderDrawColor(cam->r, 240, 240, 240, 255);
    SDL_SetRenderDrawBlendMode(cam->r, SDL_BLENDMODE_MUL);

    for (double rnd = (rand() % 100) / 200.; rnd < 2; rnd += (rand() % 100) / 200.) {
        int hg = rnd * (plH / 2);
        int y = centerY - hg * 1.5;
        SDL_Ellipse(cam->r, centerX, y, plW * .7, hg, 0, 0, 1, 1); // top half
        SDL_Ellipse(cam->r, centerX, y - 1, plW * .7, hg, 0, 0, 1, 1);

        hg = (2 - rnd) * (plH / 2);
        y = centerY + hg * 1.5;
        SDL_Ellipse(cam->r, centerX, y, plW * .7, hg, 1, 1, 0, 0); // bottom half
        SDL_Ellipse(cam->r, centerX, y + 1, plW * .7, hg, 1, 1, 0, 0);
    }

    SDL_SetRenderTarget(cam->r, texture);
    SDL_SetTextureBlendMode(planet, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyF(cam->r, planet, NULL, NULL);
//    if (first)
//        save_texture("pl_3.png", cam->r, planet);
    SDL_DestroyTexture(planet);

//    if (first)
//        save_texture("pl_4.png", cam->r, texture);
    // ring, sprednji del
    SDL_SetRenderDrawBlendMode(cam->r, SDL_BLENDMODE_BLEND);
    if (ring) {
        int count = 0;
        for (int i = 0; i < ringW; ++i) {
            if (i == holeInRing) {
                i += holeInRing_size;
                continue;
            }
            int color = *clr.at_index(count++);
            SDL_SetRenderDrawColor(cam->r, color, color, color, 255);
            SDL_Ellipse(cam->r, centerX, centerY, w / 2 - (ringDist + i), h / 4 - ((ringDist + i) / 2), 0, 0, 1, 1);
        }
    }

//    if (first)
//        save_texture("pl_5.png", cam->r, texture);
//    first = false;

    // -------- nazaj render target
    SDL_SetRenderTarget(cam->r, orgTarget);
    SDL_SetRenderDrawBlendMode(cam->r, orgBlend);
}

void Planet::render(Camera *cam) {
    if (texture == nullptr) {
        cout << "texture not initialized @ Planet::render!\n";
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