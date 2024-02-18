#include "game/game.h"

string controlls[8] = {
    "DE",
    "S",
    "S",
    "AQ",
    "AE",
    "W",
    "W",
    "DQ",
};

double map(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Generator::init(Game *_g) {
    g = _g;
}
void Generator::newPlayerAt(Point transform) {
    FastCont<int> ids, thrsId;
    int centerId;

    double mult = 1. / 3.;
    double spring_hardness = 10000, spring_dampness = 30;

    // POINTS
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

    Point min = {p[0][0], p[0][1]}, max = {p[0][0], p[0][1]};
    for (int i = 0; i < 8; ++i) {
        if (min.x > p[i][0]) min.x = p[i][0];
        if (min.y > p[i][1]) min.y = p[i][1];

        if (max.x < p[i][0]) max.x = p[i][0];
        if (max.y < p[i][1]) max.y = p[i][1];
    }

    double norm[8][2] = {
        {map(1, min.x, max.x, 0, 1), map(1, min.y, max.y, 0, 1)},
        {map(0, min.x, max.x, 0, 1), map(2, min.y, max.y, 0, 1)},
        {map(1, min.x, max.x, 0, 1), map(3, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(3, min.y, max.y, 0, 1)},
        {map(3, min.x, max.x, 0, 1), map(2, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(1, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(0.3, min.y, max.y, 0, 1)},
        {map(1, min.x, max.x, 0, 1), map(0.3, min.y, max.y, 0, 1)},
    };

    // THRUSTERS
    // id1, id2, rot
    double t[8][4] = {
        {1, 4, 0},    // R
        {1, 4, -PIh}, // BW
        {4, 1, PIh},  // BW
        {4, 1, 0},    // L
        {6, 7, 0},    // L
        {6, 5, 0},    // FW
        {7, 0, 0},    // FW
        {7, 6, 0},    // R
    };

    for (int i = 0; i < 8; ++i) {
        int id = g->phisics.createNewPoint(p[i][0] * mult + transform.x, p[i][1] * mult + transform.y, 10, 0);
        ids.push_back(id);
    }

    // nastavimo center (virtualna tocka -> povprecje vseh)
    centerId = g->phisics.createNewPoint(0, 0, 0, -1, 0, 0);
    g->phisics.points.at_id(centerId)->setVirtual(true);
    for (int i = 0; i < 8; ++i) {
        g->phisics.points.at_id(centerId)->virtAvgPoints.push_back(*ids.at_index(i));
    }

    // linke med vsemi
    for (int i = 0; i < 8; ++i) {
        g->phisics.createNewLinkBetween(*ids.at_index(i), *ids.at_index((i + 1) % 8), spring_hardness, spring_dampness);
    }
    // pa se vmes
    g->phisics.createNewLinkBetween(*ids.at_index(7), *ids.at_index(5), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(0), *ids.at_index(6), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(1), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(2), *ids.at_index(0), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(2), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(3), *ids.at_index(0), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(3), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(4), *ids.at_index(0), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(0), *ids.at_index(5), spring_hardness, spring_dampness);

    // thr
    int pntTmp[4] = {*ids.at_index(5), *ids.at_index(6), *ids.at_index(7), *ids.at_index(0)};
    int fuelId = g->phisics.createNewFuelContainer(1, .03, pntTmp, 1, 1.5, 10e3);
    for (int i = 0; i < 8; ++i) {
        int id = g->phisics.createNewThrOn(t[i][0], t[i][1], t[i][2], .01, 1);

        g->phisics.rocketThrs.at_id(id)->initPs(.05, 6, PI, .5, .3, 255, 255, 255);
        g->phisics.rocketThrs.at_id(id)->ps.setSpawnInterval(.01);
        g->phisics.rocketThrs.at_id(id)->ps.setRandomises(PI / 10, 1, .1);

        g->phisics.rocketThrs.at_id(id)->setFuelSource(fuelId);

        for (int j = 0; j < controlls[i].length(); ++j) {
            g->phisics.rocketThrs.at_id(id)->controlls[j] = controlls[i][j];
        }

        thrsId.push_back(id);
    }

    Point p_min, p_max;

    p_min = {p[0][0], p[0][1]}, p_max = p_min;
    for (int i = 1; i < 8; ++i) {

        if (p_min.x > p[i][0])
            p_min.x = p[i][0];
        if (p_min.y > p[i][1])
            p_min.y = p[i][1];

        if (p_max.x < p[i][0])
            p_max.x = p[i][0];
        if (p_max.y < p[i][1])
            p_max.y = p[i][1];
    }

    PhTexture tmpTexture;
    int tmpTid = g->phisics.textures.push_back(tmpTexture);
    PhTexture *tx = g->phisics.textures.at_id(tmpTid);

    tx->setTexture(&g->cam, "media/astronaut.png");
    for (int i = 0; i < 8; ++i) {
        int i2 = (i+1)%8;
        Point normA = {norm[i][0], norm[i][1]};
        Point normB = {norm[i2][0], norm[i2][1]};
        Point normC = {.5, .5};
        tx->push_indicie(*ids.at_index(i), *ids.at_index(i2), centerId, normA, normB, normC);
    }
}