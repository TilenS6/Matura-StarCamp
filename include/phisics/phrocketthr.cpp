#include "phisics/phisics.h"

PhRocketThr::PhRocketThr() {
    psActive = false;
    for (int i = 0; i < 8; i++)
        controlls[i] = 0;
}
void PhRocketThr::init(PhWorld *_w, int attachedTo, int facing, double _dirOffset = 0, double _fuelConsumption = 1, double _fuelForceMulti = 1) {
    w = _w;
    attachedPID = attachedTo;
    facingPID = facing;
    power = 0;
    dirOffset = _dirOffset;

    fuelConsumption = _fuelConsumption;
    fuelForceMulti = _fuelForceMulti;
    fuelContId = -1;
}
void PhRocketThr::setFuelSource(int id) {
    fuelContId = id;
}
void PhRocketThr::relocate(int attachedTo, int facing) {
    attachedPID = attachedTo;
    facingPID = facing;
}
void PhRocketThr::setState(double koef) {
    if (fuelContId == -1) {
        cout << "W: @ phRocketThr.cpp: setState (fuel source not set)\n";
    }
    if (koef > 1) koef = 1;
    else if (koef < 0) koef = 0;
    power = koef;
}
void PhRocketThr::update(double dt) {
    if (fuelContId == -1)
        return;
    PhPoint *p1 = w->points.at_id(attachedPID);
    if (p1 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (attachedPID is non-existant)\n";
        return;
    }
    PhPoint *p2 = w->points.at_id(facingPID);
    if (p2 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (facingPID is non-existant)\n";
        return;
    }

    Point attached = p1->getPos();
    Point facing = p2->getPos();
    double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI + dirOffset;

    double koef;
    double currentThrust = (w->fuelConts.at_id(fuelContId)->take(fuelConsumption * dt * power, &koef) * fuelForceMulti) / dt;

    p1->force += {cos(dir) *currentThrust, sin(dir) *currentThrust};

    if (psActive) {
        ps.moveSpawner(attached, dir + PI);
        ps.update(dt, koef, p1->currentSpeed);
    }
}
void PhRocketThr::render(Camera *cam) {
    // if (currentThrust == 0) return;
    PhPoint *p1 = w->points.at_id(attachedPID);
    if (p1 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (attachedPID is non-existant)\n";
        return;
    }
    PhPoint *p2 = w->points.at_id(facingPID);
    if (p2 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (facingPID is non-existant)\n";
        return;
    }

    if (helpers) {
        Point attached = p1->getPos();
        Point facing = p2->getPos();
        double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI + dirOffset;
        Rectng rect;
        rect.a = attached;
        rect.a.x -= .05;
        rect.a.y += .05;
        rect.dimensions = { .1, .1 };
        rect.render(cam);

        Line whereThr;
        whereThr.a = attached;
        whereThr.b = attached;
        whereThr.b.x += .3 * cos(dir);
        whereThr.b.y += .3 * sin(dir);
        whereThr.render(cam);
    }

    if (psActive) {
        ps.render(cam);
    }
}

void PhRocketThr::initPs(double size, double speed, double dir, double vel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu) {
    ps.create({ 0, 0 }, size, speed, dir, vel_mult_per_second, rem_life_seconds, red, grn, blu);
    ps.setSpawnInterval(0.05);
    ps.spawning = true;
    psActive = true;
}