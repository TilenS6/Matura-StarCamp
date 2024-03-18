#include "game.h"


Inventory::Inventory() {
    selected = 0;
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        inv[i] = {none, 0};
    }
}

void Inventory::render(Camera *cam) {
    SDL_Rect rect = {0, cam->h - 50, 50, 50};

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (i == selected)
            SDL_SetRenderDrawColor(cam->r, INVENTORY_TEXTURE_BORDER_COLOUR_SELECTED, 255);
        else
            SDL_SetRenderDrawColor(cam->r, INVENTORY_TEXTURE_BORDER_COLOUR, 255);

        int x = (cam->w - (INVENTORY_TEXTURE_SIZE + 2 * INVENTORY_TEXTURE_BORDER) * INVENTORY_SIZE) / 2;
        x += (INVENTORY_TEXTURE_SIZE + 2 * INVENTORY_TEXTURE_BORDER) * i;
        rect.x = x;

        // TODO teksture tukej renderj na 'rect'

        for (int j = 0; j < INVENTORY_TEXTURE_BORDER; ++j) {
            SDL_Rect tmpRect = {rect.x - j, rect.y - j, rect.w + 2 * j, rect.h + 2 * j};
            SDL_RenderDrawRect(cam->r, &tmpRect);
        }
    }
}

// TODO (do konca) Q=drop at mouse, pickup (radius)
