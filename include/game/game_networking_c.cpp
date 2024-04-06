#include "game/game.h"

void Game::request_initialFromServer() {
#ifdef CONSOLE_LOGGING
    cout << "- init requested...\n";
#endif
    if (client.getConnectionStatus() != 0) {
        cout << "E@ Game::requestInitialFromServer() - Server error!\n";
    }
    char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};
    client.sendData(data, sizeof(data));
}

void Game::request_updateAllFromServer() {
#ifdef CONSOLE_LOGGING
    cout << "- update all requested...\n";
#endif
    if (client.getConnectionStatus() != 0) {
        cout << "E@ Game::requestInitialFromServer() - Server error!\n";
    }
    char data[] = {NETSTD_HEADER_REQUEST, NETSTD_UPDATE_ALL};
    client.sendData(data, sizeof(data));
}

void Game::networkManagerC(Game *g) {
    cout << "Client ran...\n";
    g->netRequestTimer.interval();
    while (g->running) {
        if (!g->networkingActive) {
            Sleep(1000);
            continue;
        }
        if (g->client.getConnectionStatus() != 0) {
            cout << "client.getConnectionStatus(): " << g->client.getConnectionStatus() << endl;
            g->quitInfo = "Connection to server lost";
            g->running = false;
            return;
        }
        int ret = g->client.recieveData();
        if (ret != recieveData_NO_NEW_DATA) {
#ifdef CONSOLE_LOGGING
            cout << "Data recieved!\n";
#endif
            g->halt = true;
            while (!g->halting) {
                asm("nop");
            }
            if (g->client.recvbuflen < 2)
                continue;

            switch (g->client.recvbuf[0]) {
            case NETSTD_HEADER_DATA:
                //! NEW DATA
                switch (g->client.recvbuf[1]) {
                case NETSTD_INIT:
                    g->process_init();
                    break;
                case NETSTD_UPDATE_ALL:
                    g->process_update_all();
                    break;
                case NETSTD_DELETE:
                    g->process_deletePoints();
                    break;
                case NETSTD_PICKUP_ITEM:
                    g->process_pickup();
                    break;
                default:
                    cout << "HEADER_DATA: unknown data\n";
                    break;
                }
                break;
            case NETSTD_HEADER_REQUEST:
                //! REQUEST
                switch (g->client.recvbuf[1]) {
                case NETSTD_INIT:
                    g->send_init(-1, -1);
                    break;
                case NETSTD_UPDATE_ALL:
                    g->send_update_all(-1);
                default:
                    cout << "HEADER_REQUEST: unknown data\n";
                    break;
                }

                break;
            default:
                cout << "HEADER: unknown operation\n";
                break;
            }
            g->halt = false;
        }

        if (g->netRequestTimer.getTime() >= NETW_REQ_INTERVAL) {
#ifdef CONSOLE_LOGGING
            cout << "- request sent\n";
#endif
            g->request_updateAllFromServer();
            g->netRequestTimer.interval();
        }
    }
    g->send_bye();
    int ret;
    do {
        ret = g->client.recieveData();
    } while (ret == recieveData_NO_NEW_DATA);
    if (g->client.recvbuf[0] == NETSTD_HEADER_DATA && g->client.recvbuf[1] == NETSTD_BYE) {
        cout << "connection closed successfuly\n";
    } else {
        cout << "error while closing connection!\n";
    }
}

/*
 ! NO HEADER

 ---- username
 uint16_t len
 char c0, c1, c2...
 ---- password
 uint16_t len
 char c0, c1, c2...

*/
void Game::send_loginInfo(string username, string password) {
    char buff[MAX_BUF_LEN];
    uint64_t offset = 0;

    uint16_t len = username.length();
    writeBuff(buff, offset, len);
    for (uint16_t i = 0; i < len; ++i) {
        char c = username[i];
        writeBuff(buff, offset, c);
    }
    len = password.length();
    writeBuff(buff, offset, len);
    for (uint16_t i = 0; i < len; ++i) {
        char c = password[i];
        writeBuff(buff, offset, c);
    }

    client.sendData(buff, offset);
}

void Game::process_deletePoints() {
    int bufflen = client.recvbuflen;
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;

    uint32_t len;
    readBuff(buff, offset, len);

    for (uint32_t i = 0; i < len; ++i) {
        int id;
        readBuff(buff, offset, id);
        phisics.removePointById(id);
    }
}

void Game::send_bye() {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_REQUEST;
    buff[1] = NETSTD_BYE;
    uint64_t offset = 2;

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        client.sendData(buff, offset);
#ifdef CONSOLE_LOGGING
        cout << "- bye sent\n";
#endif
    }
}

/*
initial (cel world, vse, samo na zacetku):
    FastCont<PhPoint> points; (id )
    FastCont<PhLineObst> lineObst;
    FastCont<PhLink> links;
    FastCont<PhMuscle> muscles;
    FastCont<PhLinkObst> linkObst;
    FastCont<PhRocketThr> rocketThrs;
    // FastCont<PhWeight> weights;
    FastCont<FuelCont> fuelConts;
    double gravity_accel;
    double vel_mult_second;



update (na vsake __ sek):
    FastCont<PhPoint> points; (pos, vel)
    FastCont<PhRocketThr> rocketThrs; (power)
    FastCont<PhWeight> weights; (added weight?)
    FastCont<FuelCont> fuelConts; (currentFuel)



FastCont<__>:
    st(uint32_T), [id, data],...



packet:
    HEADER - req/data, type
    BODY - data
    TRAILER - /

*/

// prepise vse
void Game::process_init() {
    int bufflen = client.recvbuflen;
    cout << "BUFFLEN = " << bufflen << endl;
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;
    phisics.resetWorld();

    // meta
    /*
        double gravity_accel;
        double vel_mult_second;
        * unsigned long randSeed (for planet generation)
        * int planetCount (for planet generation)
    */
    double gravity_accel;
    double vel_mult_second;
    unsigned long randSeed;
    int planetCount;
    readBuff_c(buff, offset, bufflen, gravity_accel);
    readBuff_c(buff, offset, bufflen, vel_mult_second);
    readBuff_c(buff, offset, bufflen, randSeed);
    readBuff_c(buff, offset, bufflen, planetCount);

    phisics.gravity_accel = gravity_accel;
    phisics.vel_mult_second = vel_mult_second;

    gen.planets(randSeed, planetCount);

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt (if virt: uint16_t len, id1, id2,...)
        * + double velocity_x, double velocity_y
    */
    uint32_t len;
    readBuff_c(buff, offset, bufflen, len);
    cout << "points len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double x;
        double y;
        double mass;
        FastCont<int> collisionGroup;
        collisionGroup.set_memory_leak_safety(false);
        double static_koef;
        double kinetic_koef;
        bool virt;
        double velocity_x;
        double velocity_y;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, x);
        readBuff_c(buff, offset, bufflen, y);
        readBuff_c(buff, offset, bufflen, mass);

        uint32_t jn;
        readBuff_c(buff, offset, bufflen, jn);
        for (uint32_t j = 0; j < jn; ++j) {
            int tmp;
            readBuff_c(buff, offset, bufflen, tmp);
            collisionGroup.push_back(tmp);
        }

        readBuff_c(buff, offset, bufflen, static_koef);
        readBuff_c(buff, offset, bufflen, kinetic_koef);
        readBuff_c(buff, offset, bufflen, virt);

        phisics.createNewPoint(x, y, mass, collisionGroup, static_koef, kinetic_koef, id);
        phisics.points.at_id(id)->setVirtual(virt);

        if (virt) {
            uint16_t len;
            readBuff_c(buff, offset, bufflen, len);
            for (uint16_t j = 0; j < len; ++j) {
                int tmpid;
                readBuff_c(buff, offset, bufflen, tmpid);
                phisics.points.at_id(id)->virtAvgPoints.push_back(tmpid);
            }
            cout << endl;
        }

        readBuff_c(buff, offset, bufflen, velocity_x);
        readBuff_c(buff, offset, bufflen, velocity_y);

        phisics.points.at_id(id)->vel = {velocity_x, velocity_y};
    }

    // lineobst
    /*
        int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "lineObst len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double x1;
        double y1;
        double x2;
        double y2;
        int coll_group;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, x1);
        readBuff_c(buff, offset, bufflen, y1);
        readBuff_c(buff, offset, bufflen, x2);
        readBuff_c(buff, offset, bufflen, y2);
        readBuff_c(buff, offset, bufflen, coll_group);

        phisics.createNewLineObst(x1, y1, x2, y2, coll_group, id);
    }

    // links
    /*
        int PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "links len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int idA;
        int idB;
        double spring_koef;
        double damp_koef;
        double maxCompression;
        double maxStretch;
        double originalLength;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, idA);
        readBuff_c(buff, offset, bufflen, idB);
        readBuff_c(buff, offset, bufflen, spring_koef);
        readBuff_c(buff, offset, bufflen, damp_koef);
        readBuff_c(buff, offset, bufflen, maxCompression);
        readBuff_c(buff, offset, bufflen, maxStretch);
        readBuff_c(buff, offset, bufflen, originalLength);

        phisics.createNewLinkBetween(idA, idB, spring_koef, damp_koef, maxCompression, maxStretch, sqrt(originalLength), id);
    }

    // muscles
    /*
        int PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "muscles len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int idA;
        int idB;
        double spring_koef;
        double damp_koef;
        double muscle_range;
        double maxCompression;
        double maxStretch;
        double originalLength;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, idA);
        readBuff_c(buff, offset, bufflen, idB);
        readBuff_c(buff, offset, bufflen, spring_koef);
        readBuff_c(buff, offset, bufflen, damp_koef);
        readBuff_c(buff, offset, bufflen, muscle_range);
        readBuff_c(buff, offset, bufflen, maxCompression);
        readBuff_c(buff, offset, bufflen, maxStretch);
        readBuff_c(buff, offset, bufflen, originalLength);

        phisics.createNewMuscleBetween(idA, idB, spring_koef, damp_koef, muscle_range, maxCompression, maxStretch, sqrt(originalLength), id);
    }

    // linkObst
    /*
        int PhWorld::createNewLinkObst(int linkId, int collG = 0) {
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "lineObst len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int linkId;
        int collG;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, linkId);
        readBuff_c(buff, offset, bufflen, collG);
        phisics.createNewLinkObst(linkId, collG, id);
    }

    // rocketThrs
    /*
        int PhWorld::createNewThrOn(int attached, int facing, double shift_direction, double fuelConsumption = .3, double forceMult = 1) {
            * + double power
            * + int fuel_source
            * + char[8] controlls
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "rocketThr len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int attached;
        int facing;
        double shift_direction;
        double fuelConsumption;
        double forceMult;
        double power;
        int fuel_source;
        char controlls[8];

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, attached);
        readBuff_c(buff, offset, bufflen, facing);
        readBuff_c(buff, offset, bufflen, shift_direction);
        readBuff_c(buff, offset, bufflen, fuelConsumption);
        readBuff_c(buff, offset, bufflen, forceMult);
        readBuff_c(buff, offset, bufflen, power);
        readBuff_c(buff, offset, bufflen, fuel_source);

        phisics.createNewThrOn(attached, facing, shift_direction, fuelConsumption, forceMult, id);
        phisics.rocketThrs.at_id(id)->setFuelSource(fuel_source);
        phisics.rocketThrs.at_id(id)->setState(power);

        phisics.rocketThrs.at_id(id)->initPs(.05, 6, PI, .5, .3, 255, 255, 255);
        phisics.rocketThrs.at_id(id)->ps.setSpawnInterval(.01);
        phisics.rocketThrs.at_id(id)->ps.setRandomises(PI / 10, 1, .1);

        for (int j = 0; j < 8; ++j) {
            char tmp;
            readBuff_c(buff, offset, bufflen, tmp);
            // cout << "thr " << i << ": " << j << "=" << (int)tmp << endl;
            phisics.rocketThrs.at_id(id)->controlls[j] = tmp;
        }
    }

    // fuelConts
    /*
        int ID
        bool virt
        if (virt)
            uint32_t virtPLen
            int ID1, ID2...
        else
            int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "fuelCont len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        bool virt;
        readBuff_c(buff, offset, bufflen, id);
        readBuff_c(buff, offset, bufflen, virt);
        // cout << id << ". virt? " << virt << endl;

        if (virt) {
            phisics.createNewFuelContainer(0, 0, 0, 0, 0, 0, id); // ker je capacity na 0 je virtual
            // phisics.fuelConts.at_id(id)->initVirtual(&phisics.fuelConts);

            uint32_t virtLen;
            readBuff_c(buff, offset, bufflen, virtLen);
            for (uint32_t j = 0; j < virtLen; ++j) {
                int tmpVirtId;
                readBuff_c(buff, offset, bufflen, tmpVirtId);
                phisics.fuelConts.at_id(id)->virtIDs.push_back(tmpVirtId);
            }
            // cout << "virt init-an\n";
            continue;
        }

        double _capacity;
        double recharge_per_second;
        int pointIdsForWeights[4];
        double empty_kg;
        double kg_perFuelUnit;
        double Ns_perFuelUnit;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, _capacity);
        readBuff_c(buff, offset, bufflen, recharge_per_second);

        for (int i = 0; i < 4; ++i) {
            readBuff_c(buff, offset, bufflen, pointIdsForWeights[i]);
        }

        readBuff_c(buff, offset, bufflen, empty_kg);
        readBuff_c(buff, offset, bufflen, kg_perFuelUnit);
        readBuff_c(buff, offset, bufflen, Ns_perFuelUnit);

        phisics.createNewFuelContainer(_capacity, recharge_per_second, pointIdsForWeights, empty_kg, kg_perFuelUnit, Ns_perFuelUnit, id);
    }

    // textures
    /*
        (chars, ends with \0) path
        --- FastCont<PhTextureTris>
        (int) len
        (int) idA, idB, idC
        (double) normA_x, normA_y, normB...
    */
    readBuff_c(buff, offset, bufflen, len);
    cout << "textures len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
        PhTexture tmpText;

        int id;
        string path = "";
        uint32_t jn;

        // --------------------------------

        readBuff_c(buff, offset, bufflen, id);

        phisics.textures.force_import(id, tmpText);
        PhTexture *tx = phisics.textures.at_id(id);

        char c;
        do {
            readBuff_c(buff, offset, bufflen, c);
            if (c != '\0') {
                path += c;
            }
        } while (c != '\0');
        cout << path << endl;

        tx->setTexture(&grend->cam, path);

        readBuff_c(buff, offset, bufflen, jn);
        for (uint32_t j = 0; j < jn; ++j) {
            int idA, idB, idC;
            Point normA, normB, normC;

            readBuff_c(buff, offset, bufflen, idA);
            readBuff_c(buff, offset, bufflen, idB);
            readBuff_c(buff, offset, bufflen, idC);

            readBuff_c(buff, offset, bufflen, normA.x);
            readBuff_c(buff, offset, bufflen, normA.y);
            readBuff_c(buff, offset, bufflen, normB.x);
            readBuff_c(buff, offset, bufflen, normB.y);
            readBuff_c(buff, offset, bufflen, normC.x);
            readBuff_c(buff, offset, bufflen, normC.y);

            tx->push_indicie(idA, idB, idC, normA, normB, normC);
            cout << idA << "," << idB << "," << idC << ": " << normA.x << "," << normA.y << ";" << normB.x << "," << normB.y << ";" << normC.x << "," << normC.y << "," << endl;
        }
    }
#ifdef CONSOLE_LOGGING
    cout << "Init data processed\n";
    cout << "offset on " << offset << "/" << client.recvbuflen << endl;
#endif
}

// prepise del vsega
void Game::process_update_all() {
    int bufflen = client.recvbuflen;
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;

    // points
    /*
        int id
        double pos_x, pos_y
        double vel_x, vel_y
        * double added_weight
    */
    uint32_t len;
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double pos_x, pos_y;
        double vel_x, vel_y;
        double added_weight;

        readBuff(buff, offset, id);

        readBuff(buff, offset, pos_x);
        readBuff(buff, offset, pos_y);
        readBuff(buff, offset, vel_x);
        readBuff(buff, offset, vel_y);

        readBuff(buff, offset, added_weight);

        PhPoint *p = phisics.points.at_id(id);
        if (p == nullptr) {
            request_initialFromServer();
            return;
        }
        p->pos = {pos_x, pos_y};
        p->vel = {vel_x, vel_y};
        p->addedMass = added_weight;
    }

    // rocketThrs
    /*
        int id
        double power
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double power;

        readBuff(buff, offset, id);
        readBuff(buff, offset, power);

        PhRocketThr *p = phisics.rocketThrs.at_id(id);
        if (p == nullptr) {
            request_initialFromServer();
            return;
        }
        p->setState(power);
    }

    // fuelConts
    /*
        int id
        double currentFuel
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double currentFuel;

        readBuff(buff, offset, id);
        readBuff(buff, offset, currentFuel);

        FuelCont *p = phisics.fuelConts.at_id(id);
        if (p == nullptr) {
            request_initialFromServer();
            return;
        }
        p->setFuel(currentFuel);
    }
#ifdef CONSOLE_LOGGING
    cout << "Update all data processed\n";
    cout << "offset on " << offset << "/" << client.recvbuflen << endl;
#endif
}

/*
header: DATA, CONTROLS
data: (int)thrID_1, (double)power_1, ...
*/

void Game::send_updatePlayerControls() { // TODO to se lahko izvaja v posebnem threadu
    if (thrSendBuffer.size == 0) return;

    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_UPDATE_PLAYER_CONTROLS;
    uint64_t offset = 2;

    int n = thrSendBuffer.size;
    writeBuff(buff, offset, n);

    for (int i = 0; i < thrSendBuffer.size; ++i) {
        int tmp = thrSendBuffer.get_id_at_index(i);
        writeBuff(buff, offset, tmp);
        double st = *thrSendBuffer.at_index(i);
        writeBuff(buff, offset, st);
    }

    client.sendData(buff, offset);
}

// -------- inventory --------

void Game::send_drop(DroppedItem it) {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_DROP_ITEM;
    uint64_t offset = 2;

    writeBuff(buff, offset, it.pos.x);
    writeBuff(buff, offset, it.pos.y);
    writeBuff(buff, offset, it.entr.ID);
    writeBuff(buff, offset, it.entr.count);

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

void Game::process_pickup() {
    uint32_t offset = 2;

    DroppedItem it;
    readBuff(client.recvbuf, offset, it);

    // ---- processing
    // removing
    double min_distPow2 = 1, index = -1;
    for (int i = 0; i < droppedItems.size; ++i) {
        DroppedItem p = *droppedItems.at_index(i);
        if (p.entr.ID != it.entr.ID) continue;
        double tmp = distancePow2(p.pos, it.pos);
        if (min_distPow2 > tmp) {
            min_distPow2 = tmp;
            index = i;
        }
    }
    if (index != -1) droppedItems.remove_index(index);

    // najprej groupa po inv.
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (client_inventory.inv[i].ID == it.entr.ID) {
            client_inventory.inv[i].count += it.entr.count;

            if (client_inventory.inv[i].count > stackSizes[client_inventory.inv[i].ID]) {
                it.entr.count = client_inventory.inv[i].count - stackSizes[client_inventory.inv[i].ID];
                client_inventory.inv[i].count = stackSizes[client_inventory.inv[i].ID];
            }
        }
        if (it.entr.count <= 0) break;
    }
    // pol dodaja v ker drug prazn slot
    if (it.entr.count > 0) {
        for (int i = 0; i < INVENTORY_SIZE; ++i) {
            if (client_inventory.inv[i].ID == none) {
                client_inventory.inv[i] = it.entr;
                it.entr.count = 0;
                break;
            }
        }
    }

    // sending back remains (if any)
    if (it.entr.count > 0) {
        cout << "posiljam nazaj\n";
        send_drop(it);
    }

    /*
    if (offset != client.recvbuflen) {
        uint8_t tmp = 0;
        readBuff(client.recvbuf + offset, tmp, client.recvbuf);
        process_pickup();
    }
    */
}