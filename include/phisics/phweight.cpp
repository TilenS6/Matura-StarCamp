#include "phisics/phisics.h"

PhWeight::PhWeight() {
    p = -1;
    addedMass = 0.0;
}
void PhWeight::attachTo(PhWorld *_w, int p_new) {
    w = _w;
    p = p_new;
}
void PhWeight::changeWeight(double kg) {
    if (p != -1) {
        w->points.at_id(p)->addedMass = kg;
        // addedMass = kg;
        // cout <<  w->points.at_id(p)->mass  << endl;
    }
}