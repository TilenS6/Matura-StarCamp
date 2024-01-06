#include "game/game.h"

void Player::init(PhWorld *world, Keyboard *keyboard, double off_x, double off_y) {
    w = world;
    kb = keyboard;
    double mult = 1. / 3.;
    double spring_hardness = 10000, spring_dampness = 30;
    double p[8][2] = {
        {1, 1},
        {0, 2},
        {1, 3},
        {2, 3},
        {3, 2},
        {2, 1},
        {2, 0.3},
        {1, 0.3},
    };

    // id1, id2, power, rot
    double t[8][4] = {
        {1, 4, 50, 0},    // R
        {1, 4, 30, -PIh}, // BW
        {4, 1, 30, PIh},  // BW
        {4, 1, 50, 0},    // L
        {6, 7, 50, 0},    // L
        {6, 5, 100, 0},    // FW
        {7, 0, 100, 0},    // FW
        {7, 6, 50, 0},    // R
    };

    for (int i = 0; i < 8; ++i) {
        int id = w->createNewPoint(p[i][0] * mult + off_x, p[i][1] * mult + off_y, 10, 0);
        ids.push_back(id);
    }

    // nastavimo center (virtualna tocka -> povprecje vseh)
    centerId = w->createNewPoint(0, 0, 0, -1, 0, 0);
    w->points.at_id(centerId)->setVirtual(true);
    for (int i = 0; i < 8; ++i) {
        w->points.at_id(centerId)->virtAvgPoints.push_back(*ids.at_index(i));
    }

    // linke med vsemi
    for (int i = 0; i < 8; ++i) {
        w->createNewLinkBetween(*ids.at_index(i), *ids.at_index((i + 1) % 8), spring_hardness, spring_dampness);
    }
    // pa se vmes
    w->createNewLinkBetween(*ids.at_index(7), *ids.at_index(5), spring_hardness, spring_dampness);
    w->createNewLinkBetween(*ids.at_index(0), *ids.at_index(6), spring_hardness, spring_dampness);

    w->createNewLinkBetween(*ids.at_index(1), *ids.at_index(5), spring_hardness, spring_dampness);

    w->createNewLinkBetween(*ids.at_index(2), *ids.at_index(0), spring_hardness, spring_dampness);
    w->createNewLinkBetween(*ids.at_index(2), *ids.at_index(5), spring_hardness, spring_dampness);

    w->createNewLinkBetween(*ids.at_index(3), *ids.at_index(0), spring_hardness, spring_dampness);
    w->createNewLinkBetween(*ids.at_index(3), *ids.at_index(5), spring_hardness, spring_dampness);

    w->createNewLinkBetween(*ids.at_index(4), *ids.at_index(0), spring_hardness, spring_dampness);

    w->createNewLinkBetween(*ids.at_index(0), *ids.at_index(5), spring_hardness, spring_dampness);

    // thr
    for (int i = 0; i < 8; ++i) {
        int id = w->createNewThrOn(t[i][0], t[i][1], t[i][2], t[i][3]);
        thrs.push_back(id);
    }
}

void Player::update() {
    w->rocketThrs.at_id(*thrs.at_index(0))->setState(kb->get(SDL_SCANCODE_D) || kb->get(SDL_SCANCODE_E));
    w->rocketThrs.at_id(*thrs.at_index(1))->setState(kb->get(SDL_SCANCODE_S));
    w->rocketThrs.at_id(*thrs.at_index(2))->setState(kb->get(SDL_SCANCODE_S));
    w->rocketThrs.at_id(*thrs.at_index(3))->setState(kb->get(SDL_SCANCODE_A) || kb->get(SDL_SCANCODE_Q));
    w->rocketThrs.at_id(*thrs.at_index(4))->setState(kb->get(SDL_SCANCODE_A) || kb->get(SDL_SCANCODE_E));
    w->rocketThrs.at_id(*thrs.at_index(5))->setState(kb->get(SDL_SCANCODE_W));
    w->rocketThrs.at_id(*thrs.at_index(6))->setState(kb->get(SDL_SCANCODE_W));
    w->rocketThrs.at_id(*thrs.at_index(7))->setState(kb->get(SDL_SCANCODE_D) || kb->get(SDL_SCANCODE_Q));
}