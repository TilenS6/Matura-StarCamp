#include "phisics/phisics.h"

PhPoint::PhPoint(double x, double y, double _mass = 1., int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    vel = {0, 0};
    force = {0, 0};
    addedMass = 0;

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    virt = false;

    ownership = -1;

    collisionGroups.push_back(collisionGroup);
}
PhPoint::PhPoint(double x, double y, double _mass, FastCont<int> collisionGroupCont, double static_koef = 1., double kinetic_koef = .7) { // koef. for: concrete-rubber
    move(x, y);
    mass = _mass;
    vel = {0, 0};
    force = {0, 0};
    addedMass = 0;

    KoF_static = static_koef;
    KoF_kinetic = kinetic_koef;
    virt = false;

    ownership = -1;

    collisionGroups = collisionGroupCont;
}
void PhPoint::move(double x, double y) {
    pos = {x, y};
    for (int i = 0; i < touchingList.size(); ++i)
        *touchingList.at_index(i) = false;
    for (int i = 0; i < touchingLinksList.size(); ++i)
        *touchingLinksList.at_index(i) = false;
}
void PhPoint::setVirtual(bool isVirtual) {
    virt = isVirtual;
}

void PhPoint::calculateCollisions(FastCont<bool> *touchingList, int i, Line movement, Line obstacle, Line obstVel, double dt, Line *res = nullptr) {
    bool foundCollision = false;
    Point closest, tmpForce = {0, 0};
    double dot = -1;
    if (*touchingList->at_index(i)) { // se je prej ze dotikal
        Circle nowCircle;
        nowCircle.a = pos;
        nowCircle.setRadius(PHISICS_COLLISIONS_CHECK_CIRCLE_RADIUS);
        if (collisionLineCircle(obstacle, nowCircle, &closest, &dot)) {
            Point obstVelAtColl = {obstVel.a.x * (1 - dot) + obstVel.b.x * dot, obstVel.a.y * (1 - dot) + obstVel.b.y * dot};

            foundCollision = true;
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double Fs = cosLineDir * force.y - sinLineDir * force.x;

            // if point is lifted from obstacle (with some sticking force, to prevent "lifting" a point from a vertical obstacle with no input)
            if (Fs < -PHISICS_COLLISIONS_REQUIRED_LIFT_FORCE) {
                // *touchingList.at(i) = false;
                return;
            }

            // keeping only dinamic force/acceleration
            double Fd = sinLineDir * force.y + cosLineDir * force.x;
            double Ad = sinLineDir * (vel.y - obstVelAtColl.y) + cosLineDir * (vel.x - obstVelAtColl.x); // pospesek je relativen na oviro

            // friction force
            double Ff = 0;
            if (abs(Ad) > PHISICS_COLLISIONS_KINETIC_FRICTION_ACCEL_THR) { // if it is moving and is to use kinetic fritction
                Ff = Fs * KoF_kinetic;
                if (Ad > 0)
                    Ff *= -1; // point to oposite direction of velocity
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
            vel = {Ad * cosLineDir + obstVelAtColl.x, Ad * sinLineDir + obstVelAtColl.y};

            pos = closest;
        }
    } else {                                                   // se se ni dotaknil
        if (collisionLineLine(movement, obstacle, &closest)) { // ce se je dotaknil prvic
            double dx = obstacle.a.x - obstacle.b.x, dy = obstacle.a.y - obstacle.b.y, len = sqrt(dx * dx + dy * dy);
            dx = obstacle.a.x - closest.x;
            dy = obstacle.a.y - closest.y;
            double len2 = sqrt(dx * dx + dy * dy);
            dot = len2 / len;

            Point obstVelAtColl = {obstVel.a.x * (1 - dot) + obstVel.b.x * dot, obstVel.a.y * (1 - dot) + obstVel.b.y * dot};
            double lineDir = atan2(obstacle.a.y - obstacle.b.y, obstacle.a.x - obstacle.b.x);
            double sinLineDir = sin(lineDir), cosLineDir = cos(lineDir);
            double As = cosLineDir * vel.y - sinLineDir * vel.x;
            if (As < 0) return; // if it is moving from wrong direction, in same way as normal (normal collision face/side of obstacle)

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
            double Ad = sinLineDir * (vel.y - obstVelAtColl.y) + cosLineDir * (vel.x - obstVelAtColl.x);
            vel = {Ad * cosLineDir + obstVelAtColl.x, Ad * sinLineDir + obstVelAtColl.y};

            // snap on the closest point on line -> prevent fazing trough it
            pos = closest;
        }
    }

    force += tmpForce;

    if (!foundCollision && *touchingList->at_index(i)) {
        *touchingList->at_index(i) = false;
    }
}

void PhPoint::resolveCollisions(double dt, FastCont<PhLineObst> *obst, FastCont<PhLink> *links, FastCont<PhLinkObst> *linkObst, FastCont<PhPoint> *points) {
    if (virt) {
        return;
    }
    while (obst->size() > touchingList.size()) {
        touchingList.push_back(false);
    }
    while (linkObst->size() > touchingLinksList.size()) {
        touchingLinksList.push_back(false);
    }

    if (obst->size() < touchingList.size()) {
        while (obst->size() < touchingList.size()) {
            touchingList.pop_back();
        }
        for (int i = 0; i < touchingList.size(); ++i) {
            *touchingList.at_index(i) = false; // TODO to je mal tko tko, ne glih tanajboljs
        }
    }
    if (linkObst->size() < touchingLinksList.size()) {
        while (obst->size() < touchingLinksList.size()) {
            touchingLinksList.pop_back();
        }
        for (int i = 0; i < touchingLinksList.size(); ++i) {
            *touchingLinksList.at_index(i) = false;
        }
    }

    Point nextPos = pos + (vel + (force / (mass + addedMass)) * dt) * dt;
    Line movement = {pos, nextPos};
    for (int i = 0; i < obst->size(); ++i) {
        // ali je v istem collision groupu
        bool found = false;
        for (int j = 0; j < collisionGroups.size(); ++j) {
            if (*collisionGroups.at_index(j) == obst->at_index(i)->collisionGroup) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        Line obstacle = obst->at_index(i)->line;
        Line obstVel = {{0, 0}, {0, 0}};
        calculateCollisions(&touchingList, i, movement, obstacle, obstVel, dt);
    }
    for (int i = 0; i < linkObst->size(); ++i) {
        // ali je v istem collision groupu
        bool found = false;
        for (int j = 0; j < collisionGroups.size(); ++j) {
            if (*collisionGroups.at_index(j) == linkObst->at_index(i)->collisionGroup) {
                found = true;
                break;
            }
        }
        if (!found) continue;

        PhPoint *a = (points->at_id(links->at_id(linkObst->at_index(i)->linkId)->idPointA));
        PhPoint *b = (points->at_id(links->at_id(linkObst->at_index(i)->linkId)->idPointB));
        if (a == nullptr || b == nullptr) cout << "!E: ne dobim ID pointa, PhPoint.resolveCollision\n";
        Line obstacle = {a->pos, b->pos};
        Line obstVel = {a->vel, b->vel};

        // TODO to ne dela neki najbols... ampak je ok...
        Point maxObstVel = obstVel.a;
        if (maxObstVel.x < obstVel.b.x) maxObstVel.x = obstVel.b.x;
        if (maxObstVel.y < obstVel.b.y) maxObstVel.y = obstVel.b.y;

        Point avgMovement = maxObstVel * dt;
        Line movementToObst = {movement.a + avgMovement * 2, movement.b}; //? tale *2, je experimental, ni po pravilih

        Line result = {0, 0};
        calculateCollisions(&touchingLinksList, i, movementToObst, obstacle, obstVel, dt, &result); //? zakaj vcasih kr skoci cez zid, you may ask... movement je samo prediction, ne pa to kam res gre, zato se lahko malo spremeni in je pol... grozno

        // 3. Newtonov zakon
        a->force -= result.a;
        b->force -= result.b;
    }

    vel += (force / (mass + addedMass)) * dt;
    force = {0, 0};
}

void PhPoint::applyChanges(double dt) {
    if (virt) return;
    // samo za NE virtual
    pos += vel * dt;
    currentSpeed = vel;
}

void PhPoint::updateVirtual(PhWorld *world) {
    if (!virt || virtAvgPoints.size() == 0) return;
    // samo za virtual
    pos = {0, 0};
    for (int i = 0; i < virtAvgPoints.size(); ++i)
        pos += world->points.at_id(*virtAvgPoints.at_index(i))->getPos();

    pos /= virtAvgPoints.size();
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