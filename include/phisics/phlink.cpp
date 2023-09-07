#include "phisics/phisics.h"

PhLink::PhLink(PhPoint *a, PhPoint *b, double spring_koef = 50, double damp_koef = 1) {
    lenPow2 = distancePow2(a->pos, b->pos);
    lastDist = lenPow2;
    pointA = a;
    pointB = b;

    springKoef = spring_koef;
    dampKoef = damp_koef;
}
void PhLink::update(double dt) {
    double dir = atan2(pointA->pos.y - pointB->pos.y, pointA->pos.x - pointB->pos.x);
    double sinDir = sin(dir), cosDir = cos(dir);

    double dx = pointA->pos.x - pointB->pos.x, dy = pointA->pos.y - pointB->pos.y;
    double distPow2 = dx * dx + dy * dy;

    double F = springKoef * .5 * (lenPow2 - distPow2);
    double Fdamp = dampKoef * ((sqrt(lastDist) - sqrt(distPow2)) / dt);

    F += Fdamp;

    pointA->force.x += F * cosDir;
    pointA->force.y += F * sinDir;

    pointB->force.x += -F * cosDir;
    pointB->force.y += -F * sinDir;

    lastDist = distPow2;
}
void PhLink::render(Camera *cam) {
    double ax = (pointA->pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pointA->pos.y - cam->y) * cam->scale);
    double bx = (pointB->pos.x - cam->x) * cam->scale;
    double by = cam->h - ((pointB->pos.y - cam->y) * cam->scale);
    if (ax < 0 || ay < 0 || bx < 0 || by < 0 || ax >= cam->w || ay >= cam->h || bx >= cam->w || by >= cam->h) return;
    SDL_RenderDrawLine(cam->r, ax, ay, bx, by);
}