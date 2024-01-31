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

uint16_t charToScancode(char c) {
    if (c > 'Z') c -= 'a' - 'A';

    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + SDL_SCANCODE_A;
    } else if (c >= '1' && c <= '9') {
        return c - '1' + SDL_SCANCODE_1;
    } else if (c == '0') {
        return SDL_SCANCODE_0;
    }

    return SDL_SCANCODE_UNKNOWN;
}

void Player::init(PhWorld *world, Keyboard *keyboard, Camera *c, double off_x, double off_y) {
    w = world;
    kb = keyboard;
    double mult = 1. / 3.;
    double spring_hardness = 10000, spring_dampness = 30;

    // id1, id2, power, rot
    double t[8][4] = {
        {1, 4, 50, 0},    // R
        {1, 4, 30, -PIh}, // BW
        {4, 1, 30, PIh},  // BW
        {4, 1, 50, 0},    // L
        {6, 7, 50, 0},    // L
        {6, 5, 100, 0},   // FW
        {7, 0, 100, 0},   // FW
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

        w->rocketThrs.at_id(id)->initPs(.05, 6, PI, .5, .3, 255, 255, 255);
        w->rocketThrs.at_id(id)->ps.setSpawnInterval(.03);
        w->rocketThrs.at_id(id)->ps.setRandomises(PI / 10, 1, .1);

        thrs.push_back(id);
    }

    // teksture
    texture = IMG_LoadTexture(c->r, "media/astronaut.png");
    // cout << SDL_GetError() << endl;

    p_min = {p[0][0], p[0][1]}, p_max = p_min;
    p_avg = p_min;
    for (int i = 1; i < 8; ++i) {
        p_avg.x += p[i][0];
        p_avg.y += p[i][1];

        if (p_min.x > p[i][0])
            p_min.x = p[i][0];
        if (p_min.y > p[i][1])
            p_min.y = p[i][1];

        if (p_max.x < p[i][0])
            p_max.x = p[i][0];
        if (p_max.y < p[i][1])
            p_max.y = p[i][1];
    }
    p_avg /= 8;
}

void Player::update() {
    for (int i = 0; i < thrs.size; ++i) {

        string tmp = controlls[i];
        bool st = false;
        for (int j = 0; j < tmp.length(); ++j) {
            if (kb->get((SDL_Scancode)charToScancode(tmp[j]))) {
                st = true;
                break;
            }
        }
        w->rocketThrs.at_id(*thrs.at_index(i))->setState(st);
    }

    // w->rocketThrs.at_id(*thrs.at_index(0))->setState(kb->get(SDL_SCANCODE_D) || kb->get(SDL_SCANCODE_E));
    // w->rocketThrs.at_id(*thrs.at_index(1))->setState(kb->get(SDL_SCANCODE_S));
    // w->rocketThrs.at_id(*thrs.at_index(2))->setState(kb->get(SDL_SCANCODE_S));
    // w->rocketThrs.at_id(*thrs.at_index(3))->setState(kb->get(SDL_SCANCODE_A) || kb->get(SDL_SCANCODE_Q));
    // w->rocketThrs.at_id(*thrs.at_index(4))->setState(kb->get(SDL_SCANCODE_A) || kb->get(SDL_SCANCODE_E));
    // w->rocketThrs.at_id(*thrs.at_index(5))->setState(kb->get(SDL_SCANCODE_W));
    // w->rocketThrs.at_id(*thrs.at_index(6))->setState(kb->get(SDL_SCANCODE_W));
    // w->rocketThrs.at_id(*thrs.at_index(7))->setState(kb->get(SDL_SCANCODE_D) || kb->get(SDL_SCANCODE_Q));
}

void Player::render(Camera *cam) {
    SDL_Vertex vert[9];
    for (int i = 0; i < 8; ++i) {
        Point point = {p[i][0], p[i][1]};
        Point rend = w->points.at_id(*ids.at_index(i))->getRenderPos(cam);
        vert[i] = {
            {(float)rend.x, (float)rend.y}, // position on screen
            {255, 255, 255, 255},           // colour
            {(float)((point.x - p_min.x) / (p_max.x - p_min.x)), 1 - (float)((point.y - p_min.y) / (p_max.y - p_min.y))},
        };
    }
    Point p = w->points.at_id(centerId)->getRenderPos(cam);
    vert[8] = {
        {(float)p.x, (float)p.y}, // position on screen
        {255, 255, 255, 255},     // colour
        {(float)((p_avg.x - p_min.x) / (p_max.x - p_min.x)), 1 - (float)((p_avg.y - p_min.y) / (p_max.y - p_min.y))},
    };

    int ind[24] = {
        0, 1, 8, // 8 je sredina
        1, 2, 8,
        2, 3, 8,
        3, 4, 8,
        4, 5, 8,
        5, 0, 8,
        0, 5, 6,
        0, 6, 7};

    SDL_RenderGeometry(cam->r, texture, vert, 9, ind, 24);
}