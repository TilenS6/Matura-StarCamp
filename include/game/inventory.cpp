#include "inventory.h"

Inventory::Inventory() {
    selected = 0;
    lastSelected = selected;
/*
    // !TMP --
    int i = 0;
    inv[i++] = {ore_rock, 20};
    inv[i++] = {ore_iron, 20};
    inv[i++] = {building_basic, 10};
    inv[i++] = {building_fuelcont, 10};
    inv[i++] = {building_rocketthr, 10};
    inv[i++] = {building_seat, 10};
    // !-- TMP
    */
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        //// TMP
        inv[i] = {none, 0};
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

        // top-down
        if (topdown_textures[i] != NULL) SDL_DestroyTexture(text);

        topdown_textures[i] = IMG_LoadTexture(cam->r, topdownMeta[i].path.c_str());
        if (topdown_textures[i] == NULL) {
            cout << "Error while loading texture! " << topdownMeta[i].path << " is inaccessible!\n";
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

        SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 200);
        SDL_RenderFillRect(cam->r, &rect);

        if (inv[i].ID >= 0 && inv[i].ID < none) {
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
    Point p = pos.getRenderPos(cam);
    if (p.x < 0 || p.y < 0 || p.x > cam->w || p.y > cam->h) return;

    SDL_Rect rect;
    rect.w = 50;
    rect.h = 50;

    rect.x = p.x - rect.w / 2;
    rect.y = p.y - rect.h / 2;

    SDL_RenderCopy(cam->r, inventory_textures[entr.ID], NULL, &rect);
}

int Inventory::addItem(InventoryEntry entr) {
    for (int j = 0; j < INVENTORY_SIZE; ++j) {
        if (inv[j].ID == entr.ID) {
            inv[j].count += entr.count;

            if (inv[j].count > stackSizes[inv[j].ID]) {
                entr.count = inv[j].count - stackSizes[inv[j].ID];
                inv[j].count = stackSizes[inv[j].ID];
            } else {
                entr.count = 0;
            }
        }
        if (entr.count <= 0) {
            return 0;
        }
    }
    // pol dodaja v ker drug prazn slot
    if (entr.count > 0) {
        for (int j = 0; j < INVENTORY_SIZE; ++j) {
            if (inv[j].ID == none) {
                inv[j] = entr;

                if (inv[j].count > stackSizes[inv[j].ID]) {
                    entr.count = inv[j].count - stackSizes[inv[j].ID];
                    inv[j].count = stackSizes[inv[j].ID];
                } else {
                    return 0;
                }
            }
        }
    }
    return entr.count;
}

int Inventory::getAvailableByID(int id) {
    int available = 0;
    for (int k = 0; k < INVENTORY_SIZE; ++k) {
        if (inv[k].ID == id && inv[k].count > 0) {
            available += inv[k].count;
        }
    }
    return available;
}