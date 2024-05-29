#include "game.h"

OreProcessor::OreProcessor() {
    tim.interval();
    _processDuration = 1;
}

void OreProcessor::setDuration(double newDuration) {
    _processDuration = newDuration;
}
double OreProcessor::getDuration() {
    return _processDuration;
}

void OreProcessor::update(FastCont<DroppedItem> *di, Inventory *inv) {
    DroppedItem last = containing;
    InteractiveDropoffArea::update(di, inv);
    if (containing.entr.ID == ore_rock) {
        if (last.entr.ID == none) {
            tim.interval();
        }
        if (tim.getTime() >= _processDuration) {
            tim.interval();

            containing.entr.count--;
            if (containing.entr.count <= 0) {
                containing.entr.ID = none;
            }

            DroppedItem rem = containing;
            containing.entr.ID = ore_iron;
            containing.entr.count = 1;
            pickupToInv(inv); // pobere ore_iron
            containing = rem; // nastav nazaj
        }
    } else if (containing.entr.ID != none) {
        pickupToInv(inv);
    }
}
void OreProcessor::render(Camera *cam) {
    InteractiveDropoffArea::render(cam);

    Rectng tmpRct = rect;
    // tmpRct.a.y += tmpRct.dimensions.y - .2;
    tmpRct.dimensions.y = .2;
    SDL_FRect progress = tmpRct.getRenderPosF(cam);

    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255); // white bg
    SDL_RenderFillRectF(cam->r, &progress);

    if (containing.entr.ID == ore_rock) {
        SDL_FRect green = progress;
        green.w *= tim.getTime() / _processDuration;
        SDL_SetRenderDrawColor(cam->r, 0, 255, 0, 255); // green progress
        SDL_RenderFillRectF(cam->r, &green);
    }

    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 255); // black border
    SDL_RenderDrawRectF(cam->r, &progress);
}