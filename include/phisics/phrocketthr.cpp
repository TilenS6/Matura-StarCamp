#include "phisics/phisics.h"

PhRocketThr::PhRocketThr(int attachedTo, int facing, double maxThr) {
    attachedPID = attachedTo;
    facingPID = facing;
    maxThrust = maxThr;
    currentThrust = 0;
}
void PhRocketThr::relocate(int attachedTo, int facing) {
    attachedPID = attachedTo;
    facingPID = facing;
}
void PhRocketThr::setState(double koef) {
    if (koef <= 1 && koef >= 0)
        currentThrust = maxThrust * koef;
}
void PhRocketThr::update(FastCont<PhPoint> *points) {
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
    double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI;

    p1->force += {cos(dir) * currentThrust, sin(dir) * currentThrust};
}
void PhRocketThr::render(Camera *cam, FastCont<PhPoint> *points) {
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
    double dir = atan2(attached.y - facing.y, attached.x - facing.x) + PI;

    Rectangle rect;
    rect.a = attached;
    rect.a.x -= .1;
    rect.a.y += .1;
    rect.dimensions = {.2, .2};
    rect.render(cam);

    Line whereThr;
    whereThr.a = attached;
    whereThr.b = attached;
    whereThr.b.x += .3 * cos(dir);
    whereThr.b.y += .3 * sin(dir);
    whereThr.render(cam);
}