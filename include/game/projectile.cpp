#include "game.h"

Projectile::Projectile(double x, double y, double sx, double sy, double damage, int owner) : p(x, y) {
    p.vel = {sx, sy};
    _damage = damage;
    ownerID = owner;
    lifeTime = 5;
}

bool Projectile::update(double dt, Game *g, int *removedLink) {
    lifeTime -= dt;
    if (lifeTime <= 0)
        return true;

    p.applyChanges(dt);
    if (!g->serverRole) return false;

    int touchedLinkID = -1;
    Line movement = {p.getLastPos(), p.getPos()};
    for (int i = 0; i < g->phisics.links.size(); ++i) {
        int ida = g->phisics.links.at_index(i)->idPointA;
        int idb = g->phisics.links.at_index(i)->idPointB;

        Line link = {g->phisics.points.at_id(ida)->getPos(), g->phisics.points.at_id(idb)->getPos()};

        if (collisionLineLine(movement, link)) {
            touchedLinkID = g->phisics.links.get_id_at_index(i);
            break;
        }
    }

    if (touchedLinkID != -1) {
        PhLink *tlink = g->phisics.links.at_id(touchedLinkID);
        bool defeated = tlink->takeDamage(_damage);
        // cout << tlink->life << " - def: " << defeated << endl;

        if (defeated) {
            *removedLink = touchedLinkID;
            if (ownerID != -1) {
                InventoryEntry di = tlink->loot;
                tlink->loot.count = 0;
                g->send_loot(ownerID, di);
            }
        }
        return true;
    }
    return false;
}

void Projectile::render(Camera *cam) {
    double wh2 = 0.1;
    Rectng rct;
    rct.a = p.getPos();
    rct.a.x -= wh2;
    rct.a.y -= wh2;
    rct.dimensions.x = wh2;
    rct.dimensions.y = wh2;
    SDL_SetRenderDrawColor(cam->r, 255, 0, 0, 255);
    rct.render(cam);
}