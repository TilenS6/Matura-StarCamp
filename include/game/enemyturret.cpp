#include "enemyturret.h"

EnemyTurret::EnemyTurret() {
}
void EnemyTurret::init(Point position, double searchDist, double rotspeed, double startingDir) {
    pos = position;
    searchDistPow2 = searchDist * searchDist;
    rotationSpeed = rotspeed;
    currentAngle = startingDir;
    targetAngle = startingDir;

    shotColldown = SHOTCOLLDOWN;
    shotTimer = 0;
}

Point EnemyTurret::findTarget(Game *g, bool &found) {
    Point target = {0, 0};
    double distancePow2 = searchDistPow2;
    found = false;

    int len = g->phisics.links.size();
    for (int i = 0; i < len; ++i) {
        PhLink *tmp = g->phisics.links.at_index(i);

        PhPoint *pA = g->phisics.points.at_id(tmp->idPointA);
        PhPoint *pB = g->phisics.points.at_id(tmp->idPointB);

        if (pA->ownership <= -10 && pB->ownership <= -10) { // to gre po vrsti, 1. ship -10, 2. ship -11,...
            Point avg = (pA->getPos() + pB->getPos()) / 2;
            double dx = pos.x - avg.x;
            double dy = pos.y - avg.y;
            double tmpDistPow2 = dx * dx + dy * dy;
            if (tmpDistPow2 < distancePow2) {
                target = avg;
                distancePow2 = tmpDistPow2;
                found = true;
            }
        }
    }
    return target;
}
void EnemyTurret::update(double dt, Game *g) {
    bool found;
    Point targetPos = findTarget(g, found);
    if (found) {
        targetAngle = atan2(pos.y - targetPos.y, pos.x - targetPos.x) + PI;
    }

    double angDiff = targetAngle - currentAngle;
    if (angDiff > PI)
        angDiff -= PI2;
    else if (angDiff <= -PI)
        angDiff += PI2;

    currentAngle += dt * rotationSpeed * (-2 * (angDiff < 0) + 1);

    if (g->serverRole) { // shoot
        shotTimer += dt;
        if (shotTimer >= shotColldown) {
            shotTimer = shotColldown;

            if (found && angDiff <= SHOTANGLEDIFF) {
                shotTimer -= shotColldown;

                int id = g->projectiles.push_back(Projectile(pos.x, pos.y, SHOTSPEED * cos(currentAngle), SHOTSPEED * sin(currentAngle), SHOTDMG, -1));
                g->addedProjectiles.push_back(id);
            }
        }
    }
}
void EnemyTurret::render(Camera *cam) {
    Rectng rect;
    rect.a = {pos.x - .25, pos.y - .25};
    rect.dimensions = {.5, .5};
    SDL_SetRenderDrawColor(cam->r, 200, 200, 200, 255);
    rect.render(cam);

    Line l;
    l.a = pos;
    l.b = {pos.x + cos(currentAngle), pos.y + sin(currentAngle)};
    SDL_SetRenderDrawColor(cam->r, 255, 0, 0, 255);
    l.render(cam);
}