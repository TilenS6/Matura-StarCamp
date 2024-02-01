#include "phisics/phisics.h"

PhPoint::PhPoint(double x, double y, double _mass = 1., int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = {0, 0};
    force = {0, 0};
    addedMass = 0;

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    virt = false;
    collisionGroups.push_back(collisionGroup);
}
PhPoint::PhPoint(double x, double y, double _mass, FastCont<int> collisionGroupCont, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    accel = {0, 0};
    force = {0, 0};
    addedMass = 0;

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    virt = false;
    collisionGroups = collisionGroupCont;
}
void PhPoint::move(double x, double y) {
    pos = {x, y};
    for (int i = 0; i < touchingList.size; ++i)
        *touchingList.at_index(i) = false;
    for (int i = 0; i < touchingLinksList.size; ++i)
        *touchingLinksList.at_index(i) = false;
}
void PhPoint::setVirtual(bool isVirtual) {
    virt = isVirtual;
}

void PhPoint::calculateCollisions(FastCont<bool> *touchingList, int i, Line movement, Line obstacle, Line obstAccel, double dt, Line *res = nullptr) {
    bool foundCollision = false;
    Point closest, tmpForce = {0, 0};
    double dot = -1;
    if (*touchingList->at_index(i)) { // se je prej ze dotikal
        Circle nowCircle;
        nowCircle.a = pos;
        nowCircle.setRadius(max(.01, distancePow2(movement.a, movement.b)));
        if (collisionLineCircle(obstacle, nowCircle, &closest, &dot)) {
            Point obstAccelAtColl = {obstAccel.a.x * (1 - dot) + obstAccel.b.x * dot, obstAccel.a.y * (1 - dot) + obstAccel.b.y * dot};

            foundCollision = true;
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double Fs = cosLineDir * force.y - sinLineDir * force.x;

            // if point is lifted from obstacle (with some sticking force, to prevent "lifting" a point from a vertical obstacle with no input)
            if (Fs < -.1) { // TODO to zafrkava pa dela tezave na obst
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

            tmpForce = {Fx * cosLineDir, Fx * sinLineDir};
            Point forceDiff = (force - tmpForce) * -1;

            if (res != nullptr) {
                res->a.x = forceDiff.x * (1 - dot);
                res->a.y = forceDiff.y * (1 - dot);
                res->b.x = forceDiff.x * dot;
                res->b.y = forceDiff.y * dot;
            }

            // same for acceleration
            accel = {Ad * cosLineDir + obstAccelAtColl.x, Ad * sinLineDir + obstAccelAtColl.y};

            pos = closest;
        }
    } else {                                                   // se se ni dotaknil
        if (collisionLineLine(movement, obstacle, &closest)) { // ce se je dotaknil prvic
            double dx = obstacle.a.x - obstacle.b.x, dy = obstacle.a.y - obstacle.b.y, len = sqrt(dx * dx + dy * dy);
            dx = obstacle.a.x - closest.x;
            dy = obstacle.a.y - closest.y;
            double len2 = sqrt(dx * dx + dy * dy);
            dot = len2 / len;

            Point obstAccelAtColl = {obstAccel.a.x * (1 - dot) + obstAccel.b.x * dot, obstAccel.a.y * (1 - dot) + obstAccel.b.y * dot};
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double As = cosLineDir * accel.y - sinLineDir * accel.x;
            if (As < 0) return; // if it is moving from wrong direction, in same way as normal (normal collision face/side of obstacle)

            // cout << "new at " << i << endl;
            foundCollision = true;
            *touchingList->at_index(i) = true;

            // keeping only dinamic force
            double Fd = sinLineDir * force.y + cosLineDir * force.x;
            tmpForce = {Fd * cosLineDir, Fd * sinLineDir};
            Point forceDiff = (force - tmpForce) * -1;

            if (res != nullptr) {
                res->a.x = forceDiff.x * (1 - dot);
                res->a.y = forceDiff.y * (1 - dot);
                res->b.x = forceDiff.x * dot;
                res->b.y = forceDiff.y * dot;
            }

            // same for acceleration
            double Ad = sinLineDir * (accel.y - obstAccelAtColl.y) + cosLineDir * (accel.x - obstAccelAtColl.x);
            accel = {Ad * cosLineDir + obstAccelAtColl.x, Ad * sinLineDir + obstAccelAtColl.y};

            // snap on the closest point on line -> prevent fazing trough it
            pos = closest;
        }
    }

    force += tmpForce;

    if (!foundCollision && *touchingList->at_index(i)) {
        *touchingList->at_index(i) = false;
        // cout << "removed " << i << ": slided of\n";
    }
}

void PhPoint::resolveCollisions(double dt, FastCont<PhLineObst> *obst, FastCont<PhLink> *links, FastCont<PhLinkObst> *linkObst, FastCont<PhPoint> *points) {
    if (virt) {
        return;
    }
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

    Point nextPos = pos + (accel + (force / (mass + addedMass)) * dt) * dt;
    Line movement = {pos, nextPos};
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

        Line obstacle = obst->at_index(i)->line;
        Line obstAccel = {{0, 0}, {0, 0}};
        calculateCollisions(&touchingList, i, movement, obstacle, obstAccel, dt);
    }
    for (int i = 0; i < linkObst->size; ++i) {
        // ali je v istem collision groupu
        bool found = false;
        for (int j = 0; j < collisionGroups.size; ++j) {
            if (*collisionGroups.at_index(j) == linkObst->at_index(i)->collisionGroup) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        PhPoint *a = (points->at_id(links->at_id(linkObst->at_index(i)->linkId)->idPointA));
        PhPoint *b = (points->at_id(links->at_id(linkObst->at_index(i)->linkId)->idPointB));
        if (a == nullptr || b == nullptr) cout << "!E: ne dobim ID pointa, phisics > resolveCollision\n";
        Line obstacle = {a->pos, b->pos};
        Line obstAccel = {a->accel, b->accel};

        // TODO to ne dela neki najbols... ampak je ok...
        Point avgMovement = ((obstAccel.a + obstAccel.b) / 2) * dt * dt;
        Line movementToObst = {movement.a - avgMovement * 5, movement.b};

        Line result = {0, 0};
        calculateCollisions(&touchingLinksList, i, movementToObst, obstacle, obstAccel, dt, &result); // zakaj vcasih kr skoci cez zid, you may ask... movement je samo prediction, ne pa to kam res gre, zato se lahko malo spremeni in je pol... grozno

        // 3. Newtonov zakon
        a->force -= result.a;
        b->force -= result.b;
    }

    accel += (force / (mass + addedMass)) * dt;
    force = {0, 0};
}

void PhPoint::applyChanges(double dt) {
    if (virt) return;
    // samo za NE virtual
    pos += accel * dt;
    currentSpeed = accel;
}

void PhPoint::updateVirtual(PhWorld *world) {
    if (!virt) return;
    // samo za virtual
    pos = {0, 0};
    for (int i = 0; i < virtAvgPoints.size; ++i)
        pos += world->points.at_id(*virtAvgPoints.at_index(i))->getPos();

    pos /= virtAvgPoints.size;
    return;
}

void PhPoint::render(Camera *cam) {
    if (!helpers) return;
    double ax = (pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pos.y - cam->y) * cam->scale);
    int r = .003 * (mass + addedMass) * cam->scale;

    if (r < 1) r = 1;
    if (r > 100) r = 100;
    int rPow2 = r * r;
    if (ax - r > cam->w || ax + r < 0 || ay - r > cam->h || ay + r < 0) return;
    for (int y = -r; y <= r; ++y)
        for (int x = -r; x <= r; ++x)
            if (x * x + y * y <= rPow2)
                SDL_RenderDrawPoint(cam->r, ax + x, ay + y);
}

Point PhPoint::getRenderPos(Camera *cam) {
    double ax = (pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pos.y - cam->y) * cam->scale);
    return (Point){ax, ay};
}