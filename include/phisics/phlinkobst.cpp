#include "phisics/phisics.h"

PhLinkObst::PhLinkObst() {
    link = nullptr;
    collisionGroup = 0;
}

void PhLinkObst::render(Camera *cam) {
    if (link == nullptr) return;
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    link->render(cam);
}