#include "game/game.h"

void Game::requestInitialFromServer() {
#ifdef CONSOLE_LOGGING
    cout << "- init requested...\n";
#endif
    if (client.getConnectionStatus() != 0) {
        cout << "E@ Game::requestInitialFromServer() - Server error!\n";
    }
    char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};
    client.sendData(data, sizeof(data));
}

void Game::requestUpdateAllFromServer() {
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
        if (!g->networkingActive || g->client.getConnectionStatus() != 0) {
            cout << "client.getConnectionStatus(): " << g->client.getConnectionStatus() << endl;
            Sleep(1000);
            continue;
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
                default:
                    break;
                }
                break;
            case NETSTD_HEADER_REQUEST:
                //! REQUEST
                switch (g->client.recvbuf[1]) {
                case NETSTD_INIT:
                    g->send_init();
                    break;
                case NETSTD_UPDATE_ALL:
                    g->send_update_all();
                default:
                    break;
                }

                break;
            default:
                break;
            }
            g->halt = false;
        }

        if (g->netRequestTimer.getTime() >= NETW_REQ_INTERVAL) {
            g->netRequestTimer.interval();
#ifdef CONSOLE_LOGGING
            cout << "- request sent\n";
#endif
            g->requestUpdateAllFromServer();
        }
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
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;
    phisics.resetWorld();

    // meta
    /*
        double gravity_accel;
        double vel_mult_second;
    */
    double gravity_accel;
    double vel_mult_second;
    readBuff(buff, offset, gravity_accel);
    readBuff(buff, offset, vel_mult_second);
    phisics.gravity_accel = gravity_accel;
    phisics.vel_mult_second = vel_mult_second;

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt (if virt: uint16_t len, id1, id2,...)
        * + double velocity_x, double velocity_y
    */
    uint32_t len;
    readBuff(buff, offset, len);
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

        readBuff(buff, offset, id);
        readBuff(buff, offset, x);
        readBuff(buff, offset, y);
        readBuff(buff, offset, mass);

        uint32_t jn;
        readBuff(buff, offset, jn);
        for (uint32_t j = 0; j < jn; ++j) {
            int tmp;
            readBuff(buff, offset, tmp);
            collisionGroup.push_back(tmp);
        }

        readBuff(buff, offset, static_koef);
        readBuff(buff, offset, kinetic_koef);

        readBuff(buff, offset, virt);
        phisics.createNewPoint(x, y, mass, collisionGroup, static_koef, kinetic_koef, id);
        phisics.points.at_id(id)->setVirtual(virt);

        if (virt) {
            uint16_t len;
            readBuff(buff, offset, len);
            for (uint16_t j = 0; j < len; ++j) {
                int tmpid;
                readBuff(buff, offset, tmpid);
                phisics.points.at_id(id)->virtAvgPoints.push_back(tmpid);
                cout << tmpid << ",";
            }
            cout << endl;
        }

        readBuff(buff, offset, velocity_x);
        readBuff(buff, offset, velocity_y);

        phisics.points.at_id(id)->vel = {velocity_x, velocity_y};
    }

    // lineobst
    /*
        int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double x1;
        double y1;
        double x2;
        double y2;
        int coll_group;

        // --------------------------------

        readBuff(buff, offset, id);
        readBuff(buff, offset, x1);
        readBuff(buff, offset, y1);
        readBuff(buff, offset, x2);
        readBuff(buff, offset, y2);
        readBuff(buff, offset, coll_group);

        phisics.createNewLineObst(x1, y1, x2, y2, coll_group, id);
    }

    // links
    /*
        int PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    readBuff(buff, offset, len);
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

        readBuff(buff, offset, id);
        readBuff(buff, offset, idA);
        readBuff(buff, offset, idB);
        readBuff(buff, offset, spring_koef);
        readBuff(buff, offset, damp_koef);
        readBuff(buff, offset, maxCompression);
        readBuff(buff, offset, maxStretch);
        readBuff(buff, offset, originalLength);

        phisics.createNewLinkBetween(idA, idB, spring_koef, damp_koef, maxCompression, maxStretch, sqrt(originalLength), id);
    }

    // muscles
    /*
        int PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    readBuff(buff, offset, len);
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

        readBuff(buff, offset, id);
        readBuff(buff, offset, idA);
        readBuff(buff, offset, idB);
        readBuff(buff, offset, spring_koef);
        readBuff(buff, offset, damp_koef);
        readBuff(buff, offset, muscle_range);
        readBuff(buff, offset, maxCompression);
        readBuff(buff, offset, maxStretch);
        readBuff(buff, offset, originalLength);

        phisics.createNewMuscleBetween(idA, idB, spring_koef, damp_koef, muscle_range, maxCompression, maxStretch, originalLength, id);
    }

    // linkObst
    /*
        int PhWorld::createNewLinkObst(int linkId, int collG = 0) {
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int linkId;
        int collG;

        // --------------------------------

        readBuff(buff, offset, id);
        readBuff(buff, offset, linkId);
        readBuff(buff, offset, collG);
        phisics.createNewLinkObst(linkId, collG, id);
    }

    // rocketThrs
    /*
        int PhWorld::createNewThrOn(int attached, int facing, double shift_direction, double fuelConsumption = .3, double forceMult = 1) {
            * + double power
            * + int fuel_source
            * + char[8] controlls
    */
    readBuff(buff, offset, len);
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

        readBuff(buff, offset, id);
        readBuff(buff, offset, attached);
        readBuff(buff, offset, facing);
        readBuff(buff, offset, shift_direction);
        readBuff(buff, offset, fuelConsumption);
        readBuff(buff, offset, forceMult);
        readBuff(buff, offset, power);
        readBuff(buff, offset, fuel_source);

        phisics.createNewThrOn(attached, facing, shift_direction, fuelConsumption, forceMult, id);
        phisics.rocketThrs.at_id(id)->setFuelSource(fuel_source);
        phisics.rocketThrs.at_id(id)->setState(power);

        phisics.rocketThrs.at_id(id)->initPs(.05, 6, PI, .5, .3, 255, 255, 255);
        phisics.rocketThrs.at_id(id)->ps.setSpawnInterval(.01);
        phisics.rocketThrs.at_id(id)->ps.setRandomises(PI / 10, 1, .1);

        for (int i = 0; i < 8; ++i) {
            char tmp;
            readBuff(buff, offset, tmp);
            phisics.rocketThrs.at_id(id)->controlls[i] = tmp;
        }
    }

    // fuelConts
    /*
        int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double _capacity;
        double recharge_per_second;
        int pointIdsForWeights[4];
        double empty_kg;
        double kg_perFuelUnit;
        double Ns_perFuelUnit;

        // --------------------------------

        readBuff(buff, offset, id);
        readBuff(buff, offset, _capacity);
        readBuff(buff, offset, recharge_per_second);

        for (int i = 0; i < 4; ++i) {
            readBuff(buff, offset, pointIdsForWeights[i]);
        }

        readBuff(buff, offset, empty_kg);
        readBuff(buff, offset, kg_perFuelUnit);
        readBuff(buff, offset, Ns_perFuelUnit);

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
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        PhTexture tmpText;

        int id;
        string path = "";
        uint32_t jn;

        // --------------------------------

        readBuff(buff, offset, id);

        phisics.textures.force_import(id, tmpText);
        PhTexture *tx = phisics.textures.at_id(id);

        char c;
        do {
            readBuff(buff, offset, c);
            if (c != '\0') {
                path += c;
            }
        } while (c != '\0');
        cout << path << endl;

        tx->setTexture(&cam, path);

        readBuff(buff, offset, jn);
        for (uint32_t j = 0; j < jn; ++j) {
            int idA, idB, idC;
            Point normA, normB, normC;

            readBuff(buff, offset, idA);
            readBuff(buff, offset, idB);
            readBuff(buff, offset, idC);

            readBuff(buff, offset, normA.x);
            readBuff(buff, offset, normA.y);
            readBuff(buff, offset, normB.x);
            readBuff(buff, offset, normB.y);
            readBuff(buff, offset, normC.x);
            readBuff(buff, offset, normC.y);

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

        phisics.rocketThrs.at_id(id)->setState(power);
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

        phisics.fuelConts.at_id(id)->setFuel(currentFuel);
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