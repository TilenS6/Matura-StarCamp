#include "phisics/phisics.h"

PhPoint::PhPoint(double x, double y, double _mass = 1., int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = {0, 0};
    force = {0, 0};

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    collisionGroups.push_back(collisionGroup);
}
PhPoint::PhPoint(double x, double y, double _mass, FastCont<int> collisionGroupCont, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = {0, 0};
    force = {0, 0};

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    collisionGroups = collisionGroupCont;
}
void PhPoint::move(double x, double y) {
    pos = {x, y};
    for (int i = 0; i < touchingList.size; ++i)
        *touchingList.at_index(i) = false;
    for (int i = 0; i < touchingLinksList.size; ++i)
        *touchingLinksList.at_index(i) = false;
}

void PhPoint::resolveCollisions(double dt, FastCont<PhLineObst> *obst, FastCont<PhLinkObst> *linkObst) {
    while (obst->size > touchingList.size) {
        touchingList.push_back(false);
    }
    while (linkObst->size > touchingLinksList.size) {
        touchingLinksList.push_back(false);
    }

    if (obst->size < touchingList.size) {
        while (obst->size < touchingList.size) {
            touchingList.pop_back();
        }
        for (int i = 0; i < touchingList.size; ++i)
            *touchingList.at_index(i) = false; // TODO to je mal tko tko, ne glih tanajboljs
    }
    if (linkObst->size < touchingLinksList.size) {
        while (obst->size < touchingLinksList.size) {
            touchingLinksList.pop_back();
        }
        for (int i = 0; i < touchingLinksList.size; ++i)
            *touchingLinksList.at_index(i) = false;
    }

    Point nextPos = pos + (accel + (force / mass) * dt) * dt;
    Line movement = {pos, nextPos};
<<<<<<< HEAD
=======
    Point closest;
>>>>>>> dev
    for (int i = 0; i < obst->size; ++i) {
        // ali je v istem collision groupu
        bool found = false;
        for (int j = 0; j < collisionGroups.size; ++j) {
            if (*collisionGroups.at_index(j) == obst->at_index(i)->collisionGroup) {
                found = true;
                break;
            }
        }
        if (!found) continue;

<<<<<<< HEAD
        Line obstacle = obst->at_index(i)->line;
        Line obstAccel = {{0, 0}, {0, 0}};
        calculateCollisions(&touchingList, i, movement, obstacle, obstAccel, dt);
=======
        bool foundCollision = false;
        Line obsticle = obst->at_index(i)->line;
        Line obstAccel = {{0, 0}, {0, 0}};
        if (*touchingList.at_index(i)) { // se je prej ze dotikal
            Circle nowCircle;
            nowCircle.a = pos;
            nowCircle.setRadius(max(.01, distancePow2(movement.a, movement.b)));
            double dot;
            if (collisionLineCircle(obst->at_index(i)->line, nowCircle, &closest, &dot)) {
                foundCollision = true;
                double lineDir = atan2(obsticle.a.y - obsticle.b.y, obsticle.a.x - obsticle.b.x);
                double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
                double Fs = cosLineDir * force.y - sinLineDir * force.x;

                // if point is lifted from obsticle (with some sticking force, to prevent "lifting" a point from a vertical obsticle with no input)
                if (Fs < -.001) {
                    // *touchingList.at(i) = false;
                    // cout << "removed " << i << endl;
                    continue;
                }

                // keeping only dinamic force/acceleration
                double Fd = sinLineDir * force.y + cosLineDir * force.x;
                double Ad = sinLineDir * accel.y + cosLineDir * accel.x;

                // friction force
                double Ff = 0;
                if (abs(Ad) > .02) { // if it is moving and is to use kinetic fritction
                    Ff = Fs * KoF_kinetic;
                    if (Ad > 0) Ff *= -1; // point to oposite direction of acceleration
                } else {
                    if (abs(Fd) <= KoF_static * Fs) {
                        Ff = -Fd;
                        Ad = 0;
                    }
                }
                double Fx = Fd + Ff;

                force = {Fx * cosLineDir, Fx * sinLineDir};

                // same for acceleration
                accel = {Ad * cosLineDir, Ad * sinLineDir};

                pos = closest;
            }
        } else {                                                   // se se ni dotaknil
            if (collisionLineLine(movement, obsticle, &closest)) { // ce se je dotaknil prvic
                double lineDir = atan2(obsticle.a.y - obsticle.b.y, obsticle.a.x - obsticle.b.x);
                double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
                double As = cosLineDir * accel.y - sinLineDir * accel.x;
                if (As < 0) continue; // if it is moving from wrong direction, in same way as normal (normal collision face/side of obsticle)

                // cout << "new at " << i << endl;
                foundCollision = true;
                *touchingList.at_index(i) = true;

                // keeping only dinamic force
                double Fd = sinLineDir * force.y + cosLineDir * force.x;
                force = {Fd * cosLineDir, Fd * sinLineDir};

                // same for acceleration
                double Ad = sinLineDir * accel.y + cosLineDir * accel.x;
                accel = {Ad * cosLineDir, Ad * sinLineDir};

                // snap on the closest point on line -> prevent fazing trough it
                pos = closest;
            }
        }

        if (!foundCollision && *touchingList.at_index(i)) {
            *touchingList.at_index(i) = false;
            // cout << "removed " << i << ": slided of\n";
        }
>>>>>>> dev
    }

    accel += (force / mass) * dt;
    force = {0, 0};
<<<<<<< HEAD
}

void PhPoint::calculateCollisions(FastCont<bool> *touchingList, int i, Line movement, Line obstacle, Line obstAccel, double dt) {
    bool foundCollision = false;
    Point closest;
    if (*touchingList->at_index(i)) { // se je prej ze dotikal
        Circle nowCircle;
        nowCircle.a = pos;
        nowCircle.setRadius(max(.01, distancePow2(movement.a, movement.b)));
        double dot;
        if (collisionLineCircle(obstacle, nowCircle, &closest, &dot)) {
            Point obstAccelAtColl = {obstAccel.a.x * dot + obstAccel.b.x * (1 - dot), obstAccel.a.y * dot + obstAccel.b.y * (1 - dot)};

            foundCollision = true;
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double Fs = cosLineDir * force.y - sinLineDir * force.x;

            // if point is lifted from obstacle (with some sticking force, to prevent "lifting" a point from a vertical obstacle with no input)
            if (Fs < -.001) {
                // *touchingList.at(i) = false;
                // cout << "removed " << i << endl;
                return;
            }

            // keeping only dinamic force/acceleration
            double Fd = sinLineDir * force.y + cosLineDir * force.x;
            double Ad = sinLineDir * (accel.y - obstAccelAtColl.y) + cosLineDir * (accel.x - obstAccelAtColl.x); // pospesek je relativen na oviro

            // friction force
            double Ff = 0;
            if (abs(Ad) > .02) { // if it is moving and is to use kinetic fritction
                Ff = Fs * KoF_kinetic;
                if (Ad > 0)
                    Ff *= -1; // point to oposite direction of acceleration
            } else {
                if (abs(Fd) <= KoF_static * Fs) {
                    Ff = -Fd;
                    Ad = 0;
                }
            }
            double Fx = Fd + Ff;

            force = {Fx * cosLineDir, Fx * sinLineDir};

            // same for acceleration
            accel = {Ad * cosLineDir, Ad * sinLineDir};

            pos = closest;
        }
    } else {                                                   // se se ni dotaknil
        if (collisionLineLine(movement, obstacle, &closest)) { // ce se je dotaknil prvic
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double As = cosLineDir * accel.y - sinLineDir * accel.x;
            if (As < 0) return; // if it is moving from wrong direction, in same way as normal (normal collision face/side of obstacle)

            // cout << "new at " << i << endl;
            foundCollision = true;
            *touchingList->at_index(i) = true;

            // keeping only dinamic force
            double Fd = sinLineDir * force.y + cosLineDir * force.x;
            force = {Fd * cosLineDir, Fd * sinLineDir};

            // same for acceleration
            double Ad = sinLineDir * accel.y + cosLineDir * accel.x;
            accel = {Ad * cosLineDir, Ad * sinLineDir};

            // snap on the closest point on line -> prevent fazing trough it
            pos = closest;
        }
    }

    if (!foundCollision && *touchingList->at_index(i)) {
        *touchingList->at_index(i) = false;
        // cout << "removed " << i << ": slided of\n";
    }
=======
>>>>>>> dev
}

void PhPoint::applyChanges(double dt) {
    pos += accel * dt;
}

void PhPoint::render(Camera *cam) {
    double ax = (pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pos.y - cam->y) * cam->scale);
    int r = .05 * mass * cam->scale;

    if (r < 1) r = 1;
    if (r > 100) r = 100;
<<<<<<< HEAD

    int rPow2 = r * r;
    if (ax - r > cam->w || ax + r < 0 || ay - r > cam->h || ay + r < 0) return;

=======
    int rPow2 = r * r;
    if (ax - r > cam->w || ax + r < 0 || ay - r > cam->h || ay + r < 0) return;
>>>>>>> dev
    for (int y = -r; y <= r; ++y)
        for (int x = -r; x <= r; ++x)
            if (x * x + y * y <= rPow2)
                SDL_RenderDrawPoint(cam->r, ax + x, ay + y);
}