#include "phisics/phisics.h"

PhLineObst::PhLineObst() {
    collisionGroup = 0;
}

void PhLineObst::render(Camera *cam) {
    double ax = (line.a.x - cam->x) * cam->scale;
    double ay = cam->h - ((line.a.y - cam->y) * cam->scale);
    double bx = (line.b.x - cam->x) * cam->scale;
    double by = cam->h - ((line.b.y - cam->y) * cam->scale);
    if (ax < 0 || ay < 0 || bx < 0 || by < 0 || ax >= cam->w || ay >= cam->h || bx >= cam->w || by >= cam->h) return;
    SDL_RenderDrawLine(cam->r, ax, ay, bx, by);
}