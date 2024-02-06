#include "game/game.h"

void Game::requestInitialFromServer() {
    if (client.getConnectionStatus() != 0) {
        cout << "E@ Game::requestInitialFromServer() - Server error!\n";
    }
    char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};
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
            cout << "Data recieved!\n";
            g->halt = true;
            while (!g->halting) {
                asm("nop");
            }
            if (g->client.recvbuflen < 2) continue;

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
            cout << "- request sent\n";

            // TODO send data + request new data
            g->requestInitialFromServer(); //! TEMP
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
    double accel_mult_second;



update (na vsake __ sek):
    FastCont<PhPoint> points; (pos, accel)
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
#define readBuff(buff, offset, a)         \
    memcpy(&a, buff + offset, sizeof(a)); \
    offset += sizeof(a);

void Game::process_init() {
    int bufflen = client.recvbuflen;
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;
    phisics.resetWorld();

    // meta
    /*
        double gravity_accel;
        double accel_mult_second;
    */
    double gravity_accel;
    double accel_mult_second;
    readBuff(buff, offset, gravity_accel);
    readBuff(buff, offset, accel_mult_second);
    phisics.gravity_accel = gravity_accel;
    phisics.accel_mult_second = accel_mult_second;

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt
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

        readBuff(buff, offset, velocity_x);
        readBuff(buff, offset, velocity_y);

        phisics.createNewPoint(x, y, mass, collisionGroup, static_koef, kinetic_koef, id);
        phisics.points.at_id(id)->setVirtual(virt);
        phisics.points.at_id(id)->accel = {velocity_x, velocity_y};
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
    */
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        int attached;
        int facing;
        double shift_direction;
        double fuelConsumption;
        double forceMult;

        // --------------------------------

        readBuff(buff, offset, id);
        readBuff(buff, offset, attached);
        readBuff(buff, offset, facing);
        readBuff(buff, offset, shift_direction);
        readBuff(buff, offset, fuelConsumption);
        readBuff(buff, offset, forceMult);
        phisics.createNewThrOn(attached, facing, shift_direction, fuelConsumption, forceMult, id);
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
#ifdef CONSOLE_LOGGING
    cout << "offset on " << offset << "/" << client.recvbuflen << endl;
    cout << "Init data processed\n";
#endif
}

// prepise del vsega
void Game::process_update_all() {
    // TODO
    cout << "TODO process_update_all()";
}
