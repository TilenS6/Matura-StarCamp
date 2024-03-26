#include "game.h"

Inventory::Inventory() {
    selected = 0;
    lastSelected = selected;

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        inv[i] = {i, 1}; //! todo tmp (tale i)
    }

    t.interval();
    text = NULL;
    font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
}

void Inventory::loadTextures(Camera *cam) {
    for (int i = 0; i < none; ++i) {
        if (inventory_textures[i] != NULL) SDL_DestroyTexture(text);

        inventory_textures[i] = IMG_LoadTexture(cam->r, inventoryMeta[i].path.c_str());
        if (inventory_textures[i] == NULL) {
            cout << "Error while loading texture! " << inventoryMeta[i].path << " is inaccessible!\n";
        }
    }
    for (int i = 0; i < 256; ++i) {
        if (numbers[i] != NULL) SDL_DestroyTexture(text);

        string txt = to_string(i);
        SDL_Surface *textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
        numbers[i] = SDL_CreateTextureFromSurface(cam->r, textSurface);
        SDL_FreeSurface(textSurface);
    }
}

void Inventory::render(Camera *cam) {
    if (inventory_textures[0] == NULL) loadTextures(cam);
    if (lastSelected != selected) {
        lastSelected = selected;
        if (text != NULL) SDL_DestroyTexture(text);
        text = NULL;
        if (inv[selected].ID != none) {
            t.interval();

            string txt = inventoryMeta[inv[selected].ID].name;

            SDL_Surface *textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
            text = SDL_CreateTextureFromSurface(cam->r, textSurface);
            SDL_FreeSurface(textSurface);
        }
    }

    SDL_Rect rect = {0, cam->h - 50, 50, 50};

    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (inv[i].count <= 0) inv[i].ID = none;

        int x = (cam->w - (INVENTORY_TEXTURE_SIZE + 2 * INVENTORY_TEXTURE_BORDER) * INVENTORY_SIZE) / 2;
        x += (INVENTORY_TEXTURE_SIZE + 2 * INVENTORY_TEXTURE_BORDER) * i;
        rect.x = x;

        SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 100);
        SDL_RenderFillRect(cam->r, &rect);

        if (inv[i].ID != none) {
            SDL_RenderCopy(cam->r, inventory_textures[inv[i].ID], NULL, &rect);

            if (inv[i].count >= 0 && inv[i].count <= 255) {
                SDL_Rect nmbRect;
                SDL_QueryTexture(numbers[inv[i].count], NULL, NULL, &nmbRect.w, &nmbRect.h);
                nmbRect.x = rect.x + (rect.w - nmbRect.w) * .5;
                nmbRect.y = rect.y + rect.h - nmbRect.h;
                SDL_RenderCopy(cam->r, numbers[inv[i].count], NULL, &nmbRect);
            }
        }

        if (i == selected)
            SDL_SetRenderDrawColor(cam->r, INVENTORY_TEXTURE_BORDER_COLOUR_SELECTED, 255);
        else
            SDL_SetRenderDrawColor(cam->r, INVENTORY_TEXTURE_BORDER_COLOUR, 255);

        for (int j = 0; j < INVENTORY_TEXTURE_BORDER; ++j) {
            SDL_Rect tmpRect = {rect.x - j, rect.y - j, rect.w + 2 * j, rect.h + 2 * j};
            SDL_RenderDrawRect(cam->r, &tmpRect);
        }
    }

    if (text != NULL) {
        SDL_Rect txtRect;
        SDL_QueryTexture(text, NULL, NULL, &txtRect.w, &txtRect.h);
        txtRect.x = (cam->w - txtRect.w) * .5;
        txtRect.y = cam->h - 80;

        double time = t.getTime();
        if (time > 1) time = 1;
        int alpha = (1 - time) * 255;
        SDL_SetTextureAlphaMod(text, alpha);

        SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 100 * (1 - time));
        SDL_RenderFillRect(cam->r, &txtRect);

        SDL_RenderCopy(cam->r, text, NULL, &txtRect);
    }
}

void DroppedItem::render(Camera *cam) {
    if (entr.ID < 0 || entr.ID >= none) return;
    Point p = pos.renderAt(cam);
    if (p.x < 0 || p.y < 0 || p.x > cam->w || p.y > cam->h) return;

    SDL_Rect rect;
    rect.w = 50;
    rect.h = 50;

    rect.x = p.x - rect.w / 2;
    rect.y = p.y - rect.h / 2;

    SDL_RenderCopy(cam->r, inventory_textures[entr.ID], NULL, &rect);
}