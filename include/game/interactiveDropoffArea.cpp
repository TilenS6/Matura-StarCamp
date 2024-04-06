#include "game.h"

InteractiveDropoffArea::InteractiveDropoffArea() {
    rect.a = { 0, 0 };
    rect.dimensions = { 0, 0 };

    containing.entr.ID = none;
    containing.entr.count = 0;

    rotation = 0;
    hijacked = false;

    keybindCapturingActive = false;
    thrusterKeybind = '\0';
    keybindTex = nullptr;
}
void InteractiveDropoffArea::setRect(double x, double y, double w, double h) {
    cout << "r: " << x << ", " << y << ":\t" << w << "," << h << endl;
    rect.a = { x, y };
    rect.dimensions = { w, h };
}
bool InteractiveDropoffArea::update(FastCont<DroppedItem>* di, Inventory* inv = nullptr) {
    for (int i = 0; i < di->size; ++i) {
        if (collisionPointRectangle(di->at_index(i)->pos, rect)) {
            if (containing.entr.ID == none) {
                // gre not
                containing = *di->at_index(i);
                di->remove_index(i);
                --i;
                return true;
            } else if (inv != nullptr) {
                pickupToInv(inv);

                if (containing.entr.ID == none) {
                    containing = *di->at_index(i);
                    di->remove_index(i);
                    --i;
                }
                return true;
            }
        }
    }
    return false;
}
void InteractiveDropoffArea::updateHijack(Keyboard* kb, Mouse* m, Inventory* inv, Camera* cam) {
    if (hijacked) {
        if (keybindCapturingActive) {
            char c = kb->getLastChar();
            if (c != '\0') {
                thrusterKeybind = c;
                keybindCapturingActive = false;

                if (keybindTex != nullptr) SDL_DestroyTexture(keybindTex);

                string txt = "";
                txt += c;
                TTF_Font* font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
                SDL_Surface* textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({ 255, 255, 255, 255 })); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
                keybindTex = SDL_CreateTextureFromSurface(cam->r, textSurface);
                SDL_FreeSurface(textSurface);

                SDL_QueryTexture(keybindTex, NULL, NULL, &tw, &th);
                TTF_CloseFont(font);
            }
        }
        if (kb->get(SDL_SCANCODE_R)) {
            if (containing.entr.ID != none) pickupToInv(inv);
            kb->up(SDL_SCANCODE_R);
        }
        if (kb->get(SDL_SCANCODE_Q)) {
            rotation -= 90;
            kb->up(SDL_SCANCODE_Q);
        }
        if (kb->get(SDL_SCANCODE_E)) {
            rotation += 90;
            kb->up(SDL_SCANCODE_E);
        }
        if (kb->get(SDL_SCANCODE_F)) {
            hijacked = false;
            keybindCapturingActive = false;
            kb->up(SDL_SCANCODE_F);
        }
        if (kb->get(SDL_SCANCODE_RETURN)) {
            keybindCapturingActive = true;
            kb->up(SDL_SCANCODE_RETURN);
        }
    } else {
        Point mp;
        mp.x = m->x / cam->scale + cam->x;
        mp.y = (cam->h - m->y) / cam->scale + cam->y;

        if (kb->pressedNow(SDL_SCANCODE_F) && collisionPointRectangle(mp, rect)) {
            hijacked = true;
            kb->up(SDL_SCANCODE_F);
            return;
        }
    }
}
void InteractiveDropoffArea::pickupToInv(Inventory* inv) {
    // najprej groupa po inv.
    for (int j = 0; j < INVENTORY_SIZE; ++j) {
        if (inv->inv[j].ID == containing.entr.ID) {
            inv->inv[j].count += containing.entr.count;

            if (inv->inv[j].count > stackSizes[inv->inv[j].ID]) {
                containing.entr.count = inv->inv[j].count - stackSizes[inv->inv[j].ID];
                inv->inv[j].count = stackSizes[inv->inv[j].ID];
            }
        }
        if (containing.entr.count <= 0) {
            containing.entr.ID = none;
            thrusterKeybind = '\0';
            break;
        }
    }
    // pol dodaja v ker drug prazn slot
    if (containing.entr.count > 0) {
        for (int j = 0; j < INVENTORY_SIZE; ++j) {
            if (inv->inv[j].ID == none) {
                inv->inv[j] = containing.entr;
                containing.entr.count = 0;
                containing.entr.ID = none;
                thrusterKeybind = '\0';
                break;
            }
        }
    }
}
void InteractiveDropoffArea::render(Camera* cam) {
    SDL_FRect rendRect = rect.getRenderPosF(cam);
    if (containing.entr.ID != none) {
        SDL_FPoint center;
        center.x = rendRect.w / 2.;
        center.y = rendRect.h / 2.;

        SDL_RenderCopyExF(cam->r, topdown_textures[containing.entr.ID], NULL, &rendRect, rotation, &center, SDL_FLIP_NONE);

        SDL_FRect txtRect;
        txtRect.x = rendRect.x;
        txtRect.y = rendRect.y;
        txtRect.w = tw;
        txtRect.h = th;

        SDL_RenderCopyF(cam->r, keybindTex, NULL, &txtRect);
    }

    if (keybindCapturingActive)
        SDL_SetRenderDrawColor(cam->r, 0, 255, 0, 255);
    else if (hijacked)
        SDL_SetRenderDrawColor(cam->r, 255, 0, 0, 255);
    else
        SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);

    SDL_RenderDrawRectF(cam->r, &rendRect);
}