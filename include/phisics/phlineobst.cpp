#include "phisics/phisics.h"

PhLineObst::PhLineObst(double x1, double y1, double x2, double y2, int coll_group) {
    line = {{x1, y1}, {x2, y2}};
    collisionGroup = coll_group;
}

void PhLineObst::render(Camera *cam) {
    Line l;
    l.a = {(line.a.x - cam->x) * cam->scale, cam->h - ((line.a.y - cam->y) * cam->scale)};
    l.b = {(line.b.x - cam->x) * cam->scale, cam->h - ((line.b.y - cam->y) * cam->scale)};

    Rectangle rec;
    rec.a = {0, 0};
    rec.dimensions = {(double)cam->w, (double)cam->h};
    if (!collisionLineRectangle(l, rec)) return;

    SDL_RenderDrawLine(cam->r, l.a.x, l.a.y, l.b.x, l.b.y);
}
