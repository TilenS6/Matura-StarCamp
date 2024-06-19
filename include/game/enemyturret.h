#include "game.h"
#pragma once

#define SHOTANGLEDIFF PIh /16
#define SHOTCOLLDOWN .25
#define SHOTSPEED 5
#define SHOTDMG .55

class EnemyTurret {
    Point pos;

    double searchDistPow2;

    double currentAngle, targetAngle;
    double rotationSpeed; // in rad/sec

    double shotColldown, shotTimer;

    Point findTarget(Game *, bool &);

public:
    EnemyTurret();
    void init(Point, double, double = 1, double = 0);

    void update(double, Game *);
    void render(Camera *);
};