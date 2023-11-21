#include "phisics/phisics.h"

PhLinkObst::PhLinkObst(FastCont<PhLink> *p) {
    links = p;
    linkId = -1;
    collisionGroup = 0;
}

void PhLinkObst::render(Camera *cam) {
    if (linkId == -1) return;
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    links->at_id(linkId)->render(cam);
}