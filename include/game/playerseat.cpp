#include "game.h"

void PlayerSeat::init(int onPointID, Game *_g) {
    PID = onPointID;
    // seatedID = -1;
    g = _g;
    Point here = g->phisics.points.at_id(PID)->getPos();
    btn.init(here, "Seat down", &g->grend->cam, 100);
}
/*
void PlayerSeat::seatDownPlayer() {
    if (seatedID != -1) return;

    for (int i = 0; i < g->phisics.points.size(); ++i) {
        if (g->phisics.points.at_index(i)->ownership == playerID) {
            g->phisics.points.at_index(i)->setVirtual(true);
            g->phisics.points.at_index(i)->virtAvgPoints.clear();
        }
    }
    lastPos = g->phisics.points.at_index(PID)->getPos();

    seatedID = playerID;
}
*/
void PlayerSeat::standUpCurrentPlayer() {
    /*
    if (seatedID == -1) return;

    for (int i = 0; i < g->phisics.points.size(); ++i) {
        if (g->phisics.points.at_index(i)->ownership == seatedID) {
            g->phisics.points.at_index(i)->setVirtual(false);
        }
    }

    seatedID = -1;
    */
}
bool PlayerSeat::update(double dt) {
    Point here = g->phisics.points.at_id(PID)->getPos();
    btn.pos = here;
    if (btn.update(g->playerMedian, dt, &g->kb) != onpress_notpressed) {
        return true;
    }
    return false;

    /*
    if (seatedID == -1) return;

    Point pos = g->phisics.points.at_index(PID)->getPos();
    Point diff = pos - lastPos;

    for (int i = 0; i < g->phisics.points.size(); ++i) {
        if (g->phisics.points.at_index(i)->ownership == seatedID) {
            Point tmp = g->phisics.points.at_index(i)->getPos();
            tmp += diff;
            g->phisics.points.at_index(i)->move(tmp.x, tmp.y);
        }
    }
    lastPos = pos;
    */
}

void PlayerSeat::render() {
    btn.render(&(g->grend->cam));
}