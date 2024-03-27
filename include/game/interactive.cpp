#include "game.h"

InteractiveDropoffArea::InteractiveDropoffArea() {
    rect.a = {0, 0};
    rect.dimensions = {0, 0};

    containing.entr.ID = none;
    containing.entr.count = 0;

    rotation = 0;
    hijacked = false;
}
void InteractiveDropoffArea::setRect(double x, double y, double w, double h) {
    rect.a = {x, y};
    rect.dimensions = {w, h};
}
bool InteractiveDropoffArea::update(FastCont<DroppedItem> *di, Inventory *inv = nullptr) {
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
void InteractiveDropoffArea::updateHijack(Keyboard *kb, Mouse *m, Inventory *inv, Camera *cam) {
    if (hijacked) {
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
            kb->up(SDL_SCANCODE_F);
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
void InteractiveDropoffArea::pickupToInv(Inventory *inv) {
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
                break;
            }
        }
    }
}
void InteractiveDropoffArea::render(Camera *cam) {
    SDL_Rect rendRect = rect.getRenderPos(cam);

    if (hijacked)
        SDL_SetRenderDrawColor(cam->r, 255, 0, 0, 255);
    else
        SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);

    SDL_RenderDrawRect(cam->r, &rendRect);

    if (containing.entr.ID == none) return;

    SDL_Point center;
    center.x = rendRect.w / 2;
    center.y = rendRect.h / 2;

    SDL_RenderCopyEx(cam->r, topdown_textures[containing.entr.ID], NULL, &rendRect, rotation, &center, SDL_FLIP_NONE);
}

// button --
InteractiveButton::InteractiveButton() {
    animationK = 0.;
    pos = {0., 0.};
    textT = nullptr;
    tw = 0;
    th = 0;
}
void InteractiveButton::init(Point where, string displayText, Camera *cam) {
    pos = where;
    txt = displayText;

    if (textT != nullptr) SDL_DestroyTexture(textT);

    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({255, 255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    textT = SDL_CreateTextureFromSurface(cam->r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(textT, NULL, NULL, &tw, &th);
    TTF_CloseFont(font);
}
bool InteractiveButton::update(Point playerPos, double dt, Keyboard *kb) {
    if (textT == nullptr) {
        cout << "InteractiveButton@update not inited!\n";
        return false;
    }
    double k = pow(ANIMATION_SPEED, dt);
    double target_animationK = 0;
    bool ret = false;

    Circle c;
    c.a = pos;
    c.setRadius(BUTTON_DISTANCE);
    if (collisionPointCircle(playerPos, c)) {
        target_animationK = 1;
        if (kb->pressedNow(SDL_SCANCODE_F)) {
            ret = true;
            kb->newFrame();
        }
        
    }
    animationK = animationK * k + target_animationK * (1 - k);

    return ret;
}

void InteractiveButton::render(Camera *cam) {
    if (textT == nullptr) {
        if (txt != "") {
            cout << "warn: InteractiveButton::render generating new text texture...\n";
            TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({255, 255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
            textT = SDL_CreateTextureFromSurface(cam->r, textSurface);
            SDL_FreeSurface(textSurface);

            SDL_QueryTexture(textT, NULL, NULL, &tw, &th);
            TTF_CloseFont(font);
        } else {
            cout << "InteractiveButton@render not inited!\n";
            return;
        }
    }

    double centerRectWH = 5 + 5 * animationK;

    Point tmp = pos.getRenderPos(cam);
    SDL_FRect rect;
    rect.w = tw * (animationK * .5 + .5);
    rect.h = th * (animationK * .5 + .5);
    rect.x = tmp.x - rect.w / 2.;
    rect.y = tmp.y - rect.h - 10; // 10px padding

    SDL_FRect centerRect;
    centerRect.x = tmp.x - centerRectWH / 2.;
    centerRect.y = tmp.y - centerRectWH / 2.;
    centerRect.w = centerRectWH;
    centerRect.h = centerRectWH;

    // center
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    SDL_RenderDrawRectF(cam->r, &centerRect);

    // text
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255 * animationK);
    SDL_SetTextureAlphaMod(textT, 255 * animationK);
    SDL_RenderDrawRectF(cam->r, &rect);
    SDL_RenderCopyF(cam->r, textT, NULL, &rect);
}