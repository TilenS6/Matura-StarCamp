#include "game.h"

void PlayerSeat::init(int onPointID, Game *_g) {
    PID = onPointID;
    // seatedID = -1;
    g = _g;
    Point here = g->phisics.points.at_id(PID)->getPos();
    btn.init(here, "Seat down", &g->grend->cam, 100);
}

void PlayerSeat::standUpCurrentPlayer() {
    g->send_standup();
}

bool PlayerSeat::update(double dt) {
    Point here = g->phisics.points.at_id(PID)->getPos();
    btn.pos = here;
    if (btn.update(g->playerMedian, dt, &g->kb) != onpress_notpressed) {
        return true;
    }
    return false;
}

void PlayerSeat::render() {
    btn.render(&(g->grend->cam));
}