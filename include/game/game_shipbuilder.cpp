#include "game.h"
/*
    double offsetX, offsetY
    int w, h
    (najprej po X: 0->w, pol po Y: 0->h) : {int ID, char keybind, int rotDeg}
*/
void Game::send_buildShip(BuildingBlockData *arr, double offX, double offY, int w, int h) {
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
            writeBuff(buff, offset, arr[y * w + x].ID);
            writeBuff(buff, offset, arr[y * w + x].keybind);
            writeBuff(buff, offset, arr[y * w + x].rotDeg);
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
void Game::process_buildShip(RecievedData *rec, int playerID) {
    --buildingShipID; // vsakic za 1 manj, v minusu, init.: buildingShipID = 0;
    uint32_t offset = 2;

    double offX, offY;
    int w, h;

    readBuff(rec->data, offset, offX);
    readBuff(rec->data, offset, offY);
    readBuff(rec->data, offset, w);
    readBuff(rec->data, offset, h);

    // -- building the ship
    FuelCont tmp;
    // tmp.virtIDs.set_memory_leak_safety(false);
    int virtID = phisics.createNewFuelContainer(0, 0, 0); // postane virtual

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int id;
            char keybind;
            int rotDeg;
            // readBuff(rec->data, offset, arr[y][x]);
            readBuff(rec->data, offset, id);
            readBuff(rec->data, offset, keybind);
            readBuff(rec->data, offset, rotDeg);
            if (id == none) continue;
            // dobi ID novo kreiranga fuel containerja (ali -1 ce ga ni naredu)
            int fcID = process_buildShip_placeBlock(id, offX + x * BUILDING_BLOCK_SIZE, offY + y * BUILDING_BLOCK_SIZE, rotDeg, BUILDING_BLOCK_SIZE, virtID, buildingShipID, keybind);

            if (fcID >= 0) {
                phisics.fuelConts.at_id(virtID)->virtIDs.push_back(fcID);
            }
        }
    }

    // -- merging near points
    for (int i = 0; i < phisics.points.size(); ++i) {
        Point p = phisics.points.at_index(i)->getPos();
        for (int j = i + 1; j < phisics.points.size(); ++j) {
            Point p2 = phisics.points.at_index(j)->getPos();
            // najdemo dve tocki ko sta zlo blizu
            if (distancePow2(p, p2) <= BUILDING_BLOCK_MERGEDISTANCE * BUILDING_BLOCK_MERGEDISTANCE) {
                // merge (J se brise, I ostaja in se mu dodajo zadeve)
                int IDi = phisics.points.get_id_at_index(i);
                int IDj = phisics.points.get_id_at_index(j);
                for (int k = 0; k < phisics.links.size(); ++k) {
                    PhLink *l = phisics.links.at_index(k);
                    if (l->idPointA == IDj)
                        l->idPointA = IDi;
                    if (l->idPointB == IDj)
                        l->idPointB = IDi;
                }
                for (int k = 0; k < phisics.muscles.size(); ++k) {
                    PhMuscle *l = phisics.muscles.at_index(k);
                    if (l->idPointA == IDj)
                        l->idPointA = IDi;
                    if (l->idPointB == IDj)
                        l->idPointB = IDi;
                }
                for (int k = 0; k < phisics.rocketThrs.size(); ++k) {
                    PhRocketThr *r = phisics.rocketThrs.at_index(k);
                    if (r->attachedPID == IDj)
                        r->attachedPID = IDi;
                    if (r->facingPID == IDj)
                        r->facingPID = IDi;
                }
                for (int k = 0; k < phisics.fuelConts.size(); ++k) {
                    FuelCont *f = phisics.fuelConts.at_index(k);
                    for (int m = 0; m < 4; ++m)
                        if (f->pointIDs[m] == IDj) f->pointIDs[m] = IDi;
                }
                for (int k = 0; k < phisics.weights.size(); ++k) {
                    PhWeight *w = phisics.weights.at_index(k);
                    if (w->p == IDj) w->p = IDi;
                }
                for (int k = 0; k < phisics.textures.size(); ++k) {
                    PhTexture *t = phisics.textures.at_index(k);
                    for (int m = 0; m < t->indiciesTrises.size(); ++m) {
                        PhTextureTris *tris = t->indiciesTrises.at_index(m);
                        if (tris->idA == IDj) tris->idA = IDi;
                        if (tris->idB == IDj) tris->idB = IDi;
                        if (tris->idC == IDj) tris->idC = IDi;
                    }
                }
                phisics.removePointById(IDj);
                --j;
            }
        }
    }
}

/// @return ID of newly created FuelCont (or -1 if it isn't created at all)
int Game::process_buildShip_placeBlock(int id, double offX, double offY, int rotDeg, double scale, int thrsFuelContID, int seatID, char keybind) {
    int ret = -1;
    if (id < building_basic || id >= none) {
        cout << "E: Game::process_buildShip_placeBlock... buildShip building block ID not in range!\n";
        return -1;
    }

    FastCont<int> pids;
    double cosrot = cos(rotDeg * (PI / 180.));
    double sinrot = sin(rotDeg * (PI / 180.));

    for (int i = 0; i < constructions[id].phpoints.size(); ++i) {
        Point *p = constructions[id].phpoints.at_index(i);
        double x = ((p->x - .5) * cosrot) + .5 + ((p->y - .5) * sinrot) + .5;
        double y = ((p->x - .5) * sinrot) + .5 + ((p->y - .5) * cosrot) + .5;

        int id = phisics.createNewPoint(offX + x, offY + y, 1);
        phisics.points.at_id(id)->ownership = -seatID; // *TODO hmmm
        pids.push_back(id);
    }
    for (int i = 0; i < constructions[id].links.size(); ++i) {
        LinkStr *p = constructions[id].links.at_index(i);
        int id = phisics.createNewLinkBetween(*pids.at_index(p->idA), *pids.at_index(p->idB), BUILDING_BLOCK_SPRING, BUILDING_BLOCK_DAMP);
    }
    for (int i = 0; i < constructions[id].thrs.size(); ++i) {
        RocketThrStr *p = constructions[id].thrs.at_index(i);
        int tmpid = phisics.createNewThrOn(*pids.at_index(p->ID), *pids.at_index(p->facing), 0);
        phisics.rocketThrs.at_id(tmpid)->setFuelSource(thrsFuelContID);
        phisics.rocketThrs.at_id(tmpid)->controlls[0] = keybind;
        phisics.rocketThrs.at_id(tmpid)->controlls[1] = '\0';
        phisics.rocketThrs.at_id(tmpid)->forPlayerID = -seatID;
    }
    for (int i = 0; i < constructions[id].fuelConts.size(); ++i) {
        FuelContStr *p = constructions[id].fuelConts.at_index(i);
        int arr[4] = {*pids.at_index(p->idA), *pids.at_index(p->idB), *pids.at_index(p->idC), *pids.at_index(p->idD)};
        ret = phisics.createNewFuelContainer(BUILDING_FUEL_CAPACITY, BUILDING_FUEL_RECHARGE, arr, 1, 1, 150);
    }
    if (constructions[id].seatAt != -1) {
        PlayerSeat tmpSeat;
        tmpSeat.init(*pids.at_index(constructions[id].seatAt), this);
        seats.push_back(tmpSeat);
    }
    return ret;
}