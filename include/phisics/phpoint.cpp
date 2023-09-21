#include "phisics/phisics.h"

PhPoint::PhPoint(double x, double y, double _mass = 1., int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = { 0, 0 };
    force = { 0, 0 };

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    collisionGroups.push_back(collisionGroup);
}
PhPoint::PhPoint(double x, double y, double _mass, FastCont<int> collisionGroupCont, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = { 0, 0 };
    force = { 0, 0 };

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    collisionGroups = collisionGroupCont;
}
void PhPoint::move(double x, double y) {
    pos = { x, y };
    for (int i = 0; i < touchingList.size; ++i)
        *touchingList.at_index(i) = false;
}

void PhPoint::resolveCollisions(double dt, FastCont<PhLineObst>* obst) {
    while (obst->size > touchingList.size) {
        touchingList.push_back(false);
    }
    if (obst->size < touchingList.size) {
        while (obst->size < touchingList.size) {
            touchingList.pop_back();
        }
        for (int i = 0; i < touchingList.size; ++i)
            *touchingList.at_index(i) = false;
    }

    Point nextPos = pos + (accel + (force / mass) * dt) * dt;
    Line movement = { pos, nextPos };
    Point closest;
    for (int i = 0; i < obst->size; ++i) {
        bool found = false;
        for (int j = 0; j < collisionGroups.size; ++j) {
            if (*collisionGroups.at_index(j) == obst->at_index(i)->collisionGroup) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        bool foundCollision = false;
        if (*touchingList.at_index(i)) {
            Circle nowCircle;
            nowCircle.a = pos;
            nowCircle.setRadius(max(.01, distancePow2(movement.a, movement.b)));
            if (collisionLineCircle(obst->at_index(i)->line, nowCircle, &closest)) {
                foundCollision = true;
                double lineDir = atan2(obst->at_index(i)->line.a.y - obst->at_index(i)->line.b.y, obst->at_index(i)->line.a.x - obst->at_index(i)->line.b.x);
                double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
                double Fs = cosLineDir * force.y - sinLineDir * force.x;

                // if point is lifted from obsticle (with some sticking force, preventing "lifting" a point from a vertical line without input)
                if (Fs < -.001) {
                    // *touchingList.at(i) = false;
                    // cout << "removed " << i << endl;
                    continue;
                }

                // keeping only dinamic force
                double Fd = sinLineDir * force.y + cosLineDir * force.x;
                double Ad = sinLineDir * accel.y + cosLineDir * accel.x;

                // friction force
                double Ff = 0;
                if (abs(Ad) > .02) { // if it is moving and is to use kinetic fritction over static
                    Ff = Fs * KoF_kinetic;
                    if (Ad > 0) Ff *= -1; // set it to point to oposite side of acceleration
                } else {
                    if (abs(Fd) <= KoF_static * Fs) {
                        Ff = -Fd;
                        Ad = 0;
                    }
                }
                double Fx = Fd + Ff;

                force = { Fx * cosLineDir, Fx * sinLineDir };

                // same for acceleration
                accel = { Ad * cosLineDir, Ad * sinLineDir };

                pos = closest;
            }
        } else {
            if (collisionLineLine(movement, obst->at_index(i)->line, &closest)) {
                double lineDir = atan2(obst->at_index(i)->line.a.y - obst->at_index(i)->line.b.y, obst->at_index(i)->line.a.x - obst->at_index(i)->line.b.x);
                double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
                double As = cosLineDir * accel.y - sinLineDir * accel.x;
                if (As < 0) continue; // if it is moving from wrong direction, in same way as normal (normal collision face/side of obsticle)

                // cout << "new at " << i << endl;
                foundCollision = true;
                *touchingList.at_index(i) = true;

                // keeping only dinamic force
                double Fd = sinLineDir * force.y + cosLineDir * force.x;
                force = { Fd * cosLineDir, Fd * sinLineDir };

                // same for acceleration
                double Ad = sinLineDir * accel.y + cosLineDir * accel.x;
                accel = { Ad * cosLineDir, Ad * sinLineDir };

                // snap on the closest point on line -> prevent fazing trough it
                pos = closest;
            }
        }

        if (!foundCollision && *touchingList.at_index(i)) {
            *touchingList.at_index(i) = false;
            // cout << "removed " << i << ": slided of\n";
        }
    }

    accel += (force / mass) * dt;
    force = { 0, 0 };
}

void PhPoint::applyChanges(double dt) {
    pos += accel * dt;
}

void PhPoint::render(Camera* cam) {
    double ax = (pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pos.y - cam->y) * cam->scale);
    int r = .05 * mass * cam->scale;
    if (r < 1) r = 1;
    int rPow2 = r * r;
    for (int y = -r; y <= r; ++y)
        for (int x = -r; x <= r; ++x)
            if (x * x + y * y <= rPow2)
                SDL_RenderDrawPoint(cam->r, ax + x, ay + y);
}