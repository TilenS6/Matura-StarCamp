#include "game.h"
void Game::send_buildShip(int *arr, double offX, double offY, int w, int h) {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_BUILD;
    uint64_t offset = 2;

    // writeBuff(buff, offset, it.pos.x);

    writeBuff(buff, offset, offX);
    writeBuff(buff, offset, offY);
    writeBuff(buff, offset, w);
    writeBuff(buff, offset, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            writeBuff(buff, offset, arr[y * w + x]);
        }
    }

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        client.sendData(buff, offset);
#ifdef CONSOLE_LOGGING
        cout << "- all updated data sent as server (length: " << offset << ")\n";
#endif
    }
}
void Game::process_buildShip(RecievedData *rec) {
    uint32_t offset = 2;

    double offX, offY;
    int w, h;

    readBuff(rec->data, offset, offX);
    readBuff(rec->data, offset, offY);
    readBuff(rec->data, offset, w);
    readBuff(rec->data, offset, h);

    // -- building the ship
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int id;
            // readBuff(rec->data, offset, arr[y][x]);
            readBuff(rec->data, offset, id);

            process_buildShip_placeBlock(id, offX + x * BUILDING_BLOCK_SIZE, offY + y * BUILDING_BLOCK_SIZE, BUILDING_BLOCK_SIZE);
        }
    }

    // -- mergin near points
    for (int i = 0; i < phisics.points.size; ++i) {
        Point p = phisics.points.at_index(i)->getPos();
        for (int j = i; j < phisics.points.size; ++j) {
            Point p2 = phisics.points.at_index(j)->getPos();
            // najdemo dve tocki ko sta zlo blizu
            if (distancePow2(p, p2) <= BUILDING_BLOCK_MERGEDISTANCE * BUILDING_BLOCK_MERGEDISTANCE) {
                // merge (J se brise, I ostaja in se mu dodajo zadeve)
                for (int k = 0; k < phisics.links.size; ++k) {
                    PhLink *l = phisics.links.at_index(k);
                    if (l->idPointA == j)
                        l->idPointA = i;
                    else if (l->idPointB == j)
                        l->idPointB = i;
                }
                for (int k = 0; k < phisics.muscles.size; ++k) {
                    PhMuscle *l = phisics.muscles.at_index(k);
                    if (l->idPointA == j)
                        l->idPointA = i;
                    else if (l->idPointB == j)
                        l->idPointB = i;
                }
                for (int k = 0; k < phisics.rocketThrs.size; ++k) {
                    PhRocketThr *r = phisics.rocketThrs.at_index(k);
                    if (r->attachedPID == j)
                        r->attachedPID = i;
                    else if (r->facingPID == j)
                        r->facingPID = i;
                }
                for (int k = 0; k < phisics.fuelConts.size; ++k) {
                    FuelCont *f = phisics.fuelConts.at_index(k);
                    for (int m = 0; m < 4; ++m)
                        if (f->pointIDs[m] == j) f->pointIDs[m] = i;
                }
                for (int k = 0; k < phisics.weights.size; ++k) {
                    PhWeight *w = phisics.weights.at_index(k);
                    if (w->p == j) w->p = i;
                }
                for (int k = 0; k < phisics.textures.size; ++k) {
                    PhTexture *t = phisics.textures.at_index(k);
                    for (int m = 0; m < t->indiciesTrises.size; ++m) {
                        PhTextureTris *tris = t->indiciesTrises.at_index(m);
                        if (tris->idA == j) tris->idA = i;
                        if (tris->idB == j) tris->idB = i;
                        if (tris->idC == j) tris->idC = i;
                    }
                }
            }
        }
    }
}
void Game::process_buildShip_placeBlock(int id, double offX, double offY, double scale) {
    if (id < building_basic || id >= none) {
        cout << "E: Game::process_buildShip_placeBlock... buildShip building block ID not in range!\n";
        return;
    }

    for (int i = 0; i < constructions[id].phpoints.size; ++i) {
        Point *p = constructions[id].phpoints.at_index(i);
        phisics.createNewPoint(offX + p->x, offY + p->y, 1, 0);
    }
    for (int i = 0; i < constructions[id].links.size; ++i) {
        LinkStr *p = constructions[id].links.at_index(i);
        phisics.createNewLinkBetween(p->idA, p->idB, BUILDING_BLOCK_SPRING, BUILDING_BLOCK_DAMP);
    }
    for (int i = 0; i < constructions[id].thrs.size; ++i) {
        RocketThrStr *p = constructions[id].thrs.at_index(i);
        phisics.createNewThrOn(p->ID, p->facing, 0);
    }
}