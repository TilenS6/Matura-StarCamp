#include "phisics/phisics.h"

int PhWorld::createNewPoint(double x, double y, double mass, int collisionGroup = 0) {
    PhPoint tmp(x, y, mass, collisionGroup);
    points.push_back(tmp);
    return points.size - 1;
}

void PhWorld::removePointByPosition(double x, double y, double near = .5) {
    double nearPow2 = near * near;
    double minDistPow2 = nearPow2;
    int minAt = points.size;

    for (int i = 0; i < points.size; ++i) {
        double dx = points[i].pos.x - x, dy = points[i].pos.y - y;
        double distPow2 = dx * dx + dy * dy;
        if (distPow2 < minDistPow2) {
            minDistPow2 = distPow2;
            minAt = i;
        }
    }

    if (minAt == points.size) return; // nothing is close to specified loc

    points.remove(minAt);
}

void PhWorld::removePointById(int id) {
    if (id >= points.size) return;
    points.remove(id);
}

int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2) {
    PhLineObst tmp;
    tmp.line = {{x1, y1}, {x2, y2}};
    lineObst.push_back(tmp);
    return lineObst.size - 1;
}

int PhWorld::createNewLinkBetween(int idA, int idB) {
    PhLink tmp(points.at(idA), points.at(idB));
    links.push_back(tmp);
    return links.size - 1;
}

int PhWorld::createNewMuscleBetween(int idA, int idB) {
    PhMuscle tmp(points.at(idA), points.at(idB));
    muscles.push_back(tmp);
    return muscles.size - 1;
}

void PhWorld::applyGravity() {
    for (int i = 0; i < points.size; ++i) {
        PhPoint *pt = points.at(i);
        pt->force.y -= pt->mass * gravity_accel;
    }
}

void PhWorld::update(double dt) {
    for (int i = 0; i < links.size; ++i) {
        links.at(i)->update(dt);
    }
    for (int i = 0; i < muscles.size; ++i) {
        muscles.at(i)->update(dt);
    }
    for (int i = 0; i < points.size; ++i) {
        points.at(i)->resolveCollisions(dt, &lineObst);
    }

    // Everything is planed, apply those changes

    for (int i = 0; i < points.size; ++i) {
        points.at(i)->applyChanges(dt);
    }
}

void PhWorld::render(Camera *cam) {
    SDL_SetRenderDrawColor(cam->r, 100, 100, 100, 255);
    for (int i = 0; i < links.size; ++i)
        links.at(i)->render(cam);

    SDL_SetRenderDrawColor(cam->r, 200, 100, 100, 255);
    for (int i = 0; i < muscles.size; ++i)
        muscles.at(i)->render(cam);

    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    for (int i = 0; i < points.size; ++i)
        points.at(i)->render(cam);
    for (int i = 0; i < lineObst.size; ++i)
        lineObst.at(i)->render(cam);
}