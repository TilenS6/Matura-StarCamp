#include "phisics/phisics.h"

PhRocketThr::PhRocketThr() {
    psActive = false;
}
void PhRocketThr::init(int attachedTo, int facing, double maxThr, double _dirOffset = 0) {
    attachedPID = attachedTo;
    facingPID = facing;
    maxThrust = maxThr;
    currentThrust = 0;
    dirOffset = _dirOffset;
}
void PhRocketThr::relocate(int attachedTo, int facing) {
    attachedPID = attachedTo;
    facingPID = facing;
}
void PhRocketThr::setState(double koef) {
    if (koef <= 1 && koef >= 0)
        currentThrust = maxThrust * koef;
}
void PhRocketThr::update(FastCont<PhPoint> *points, double dt) {
    PhPoint *p1 = points->at_id(attachedPID);
    if (p1 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (attachedPID is non-existant)\n";
        return;
    }
    PhPoint *p2 = points->at_id(facingPID);
    if (p2 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (facingPID is non-existant)\n";
        return;
    }

    Point attached = p1->getPos();
    Point facing = p2->getPos();
    double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI + dirOffset;

    p1->force += {cos(dir) * currentThrust, sin(dir) * currentThrust};

    if (psActive) {
        ps.moveSpawner(attached, dir + PI);
        double mult = currentThrust / maxThrust;
        ps.update(dt, mult, p1->currentSpeed);
    }
}
void PhRocketThr::render(Camera *cam, FastCont<PhPoint> *points) {
    // if (currentThrust == 0) return;
    PhPoint *p1 = points->at_id(attachedPID);
    if (p1 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (attachedPID is non-existant)\n";
        return;
    }
    PhPoint *p2 = points->at_id(facingPID);
    if (p2 == nullptr) {
        cout << "E: @ phRocketThr.cpp: update (facingPID is non-existant)\n";
        return;
    }

    if (helpers) {
        Point attached = p1->getPos();
        Point facing = p2->getPos();
        double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI + dirOffset;

        Rectangle rect;
        rect.a = attached;
        rect.a.x -= .05;
        rect.a.y += .05;
        rect.dimensions = {.1, .1};
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

void PhRocketThr::initPs(double size, double speed, double dir, double accel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu) {
    ps.create({0, 0}, size, speed, dir, accel_mult_per_second, rem_life_seconds, red, grn, blu);
    ps.setSpawnInterval(0.05);
    ps.spawning = true;
    psActive = true;
}