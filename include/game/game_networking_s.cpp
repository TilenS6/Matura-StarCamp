#include "game/game.h"
using namespace std;
std::ostream &bold_on(std::ostream &os) {
    return os << "\e[1m";
}

std::ostream &bold_off(std::ostream &os) {
    return os << "\e[0m";
}

void Game::networkManagerS(Game *g) {
    cout << bold_on << "Server ran..." << bold_off << endl;
    FastCont<int> clientIds;
    while (g->running) {
        if (!g->networkingActive || g->client.getConnectionStatus() != 0) {
            Sleep(1000);
            continue;
        }

        int id = g->server.acceptNewClient();
        if (id >= 0) { // new client
            g->halt = true;
            clientIds.push_back(id);
            cout << "Hello " << id << "!\n";
            while (!g->halting)
                asm("nop");

            g->handle_newPlayer(id);
            g->send_init(id, id);

            g->halt = false;
        }

        for (int i = 0; i < clientIds.size; ++i) {
            int id = *clientIds.at_index(i);
            int res = g->server.recieveData(id);
            switch (res) {
            case recieveData_NO_CLIENT_ERR:
            case recieveData_CONN_CLOSED_BY_CLIENT_ERR:
                clientIds.remove_index(i);
                --i;
                break;

            case recieveData_OK: {
                RecievedData *rec = g->server.getLastData(id);
                g->halt = true;
                while (!g->halting)
                    asm("nop");
                if (rec->len < 2)
                    break; // minimum header len

                // char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};

                switch (rec->data[0]) {
                case NETSTD_HEADER_DATA:
                    //! INCOMING DATA
                    switch (rec->data[1]) {
                    case NETSTD_UPDATE_PLAYER_CONTROLS:
#ifdef CONSOLE_LOGGING
                        cout << "- controlls update recieved\n";
#endif

                        g->process_updatePlayerControls(rec);
                        break;
                    default:
                        break;
                    }
                    break;
                case NETSTD_HEADER_REQUEST:
                    //! REQUEST

                    switch (rec->data[1]) {
                    case NETSTD_INIT:
#ifdef CONSOLE_LOGGING
                        cout << "- init requested\n";
#endif
                        g->send_init(id, id);
                        break;
                    case NETSTD_UPDATE_ALL:
#ifdef CONSOLE_LOGGING
                        cout << "- update all requested\n";
#endif
                        g->send_update_all(id);
                        break;
                    default:
                        break;
                    }

                    break;
                default:
                    break;
                }
                g->halt = false;

                break;
            }

            case recieveData_NO_NEW_DATA:
            default:
                break;
            }
        }
    }
}

void Game::handle_newPlayer(int playerID) {
    gen.newPlayerAt({(double)playerID, 0}, playerID);
    // TODO
#pragma message("ok, neki ne dela s 3emi")
}

// poslje vse
void Game::send_init(int network_clientId, int playerID) {
    cout << "init: clientId=" << network_clientId << ", forPlayerId=" << playerID << endl;
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_INIT;
    uint64_t offset = 2;

    // -------- BODY --------

    // meta
    /*
        double gravity_accel;
        double vel_mult_second;
        * unsigned long randSeed (for planet generation)
        * int planetCount (for planet generation)
    */
    writeBuff(buff, offset, phisics.gravity_accel);
    writeBuff(buff, offset, phisics.vel_mult_second);
    writeBuff(buff, offset, gen.PlanetGenSeed);
    writeBuff(buff, offset, gen.PlanetCount);

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt (if virt: uint16_t len, id1, id2,...)
        * + double velocity_x, double velocity_y
    */
    uint32_t len = phisics.points.size;
    memcpy(&buff[offset], &len, sizeof(len));
    offset += sizeof(len);
    for (int i = 0; i < phisics.points.size; ++i) {
        int tmpid = phisics.points.get_id_at_index(i);
        writeBuff(buff, offset, tmpid);

        Point tmp = phisics.points.at_index(i)->getPos();
        writeBuff(buff, offset, tmp.x);
        writeBuff(buff, offset, tmp.y);

        double tmp2 = phisics.points.at_index(i)->mass;
        writeBuff(buff, offset, tmp2);

        uint32_t jn = phisics.points.at_index(i)->collisionGroups.size;
        writeBuff(buff, offset, jn);
        for (int j = 0; j < jn; ++j) {
            int tmp3 = *phisics.points.at_index(i)->collisionGroups.at_index(j);
            writeBuff(buff, offset, tmp3);
        }

        tmp2 = phisics.points.at_id(i)->KoF_static;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.points.at_id(i)->KoF_kinetic;
        writeBuff(buff, offset, tmp2);

        bool tmp3 = phisics.points.at_id(i)->virt;
        writeBuff(buff, offset, tmp3);

        if (tmp3) {
            uint16_t len = phisics.points.at_index(i)->virtAvgPoints.size;
            writeBuff(buff, offset, len);
            for (uint16_t j = 0; j < len; ++j) {
                tmpid = *phisics.points.at_index(i)->virtAvgPoints.at_index(j);
                writeBuff(buff, offset, tmpid);
            }
        }

        tmp2 = phisics.points.at_id(i)->vel.x;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.points.at_id(i)->vel.y;
        writeBuff(buff, offset, tmp2);
    }

    // lineobst
    /*
        int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    */
    len = phisics.lineObst.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.lineObst.size; ++i) {
        int tmp = phisics.lineObst.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        Line tmp2 = phisics.lineObst.at_index(i)->line;
        writeBuff(buff, offset, tmp2.a.x);
        writeBuff(buff, offset, tmp2.a.y);
        writeBuff(buff, offset, tmp2.b.x);
        writeBuff(buff, offset, tmp2.b.y);

        tmp = phisics.lineObst.at_index(i)->collisionGroup;
        writeBuff(buff, offset, tmp);
    }

    // links
    /*
        int PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    len = phisics.links.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.links.size; ++i) {
        int tmp = phisics.links.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        tmp = phisics.links.at_index(i)->idPointA;
        writeBuff(buff, offset, tmp);
        tmp = phisics.links.at_index(i)->idPointB;
        writeBuff(buff, offset, tmp);

        double tmp2 = phisics.links.at_index(i)->springKoef;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.links.at_index(i)->dampKoef;
        writeBuff(buff, offset, tmp2);

        tmp2 = phisics.links.at_index(i)->maxCompression;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.links.at_index(i)->maxStretch;
        writeBuff(buff, offset, tmp2);

        tmp2 = phisics.links.at_index(i)->orgLenPow2; // !!! POW2 !!!
        writeBuff(buff, offset, tmp2);
    }

    // muscles
    /*
        int PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */
    len = phisics.muscles.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.muscles.size; ++i) {
        int tmp = phisics.muscles.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        tmp = phisics.muscles.at_index(i)->idPointA;
        writeBuff(buff, offset, tmp);
        tmp = phisics.muscles.at_index(i)->idPointB;
        writeBuff(buff, offset, tmp);

        double tmp2 = phisics.muscles.at_index(i)->springKoef;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.muscles.at_index(i)->dampKoef;
        writeBuff(buff, offset, tmp2);

        tmp2 = (sqrt(phisics.muscles.at_index(i)->maxLenPow2) / sqrt(phisics.muscles.at_index(i)->orgLenPow2)) - 1;
        writeBuff(buff, offset, tmp2);

        tmp2 = phisics.muscles.at_index(i)->maxCompression;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.muscles.at_index(i)->maxStretch;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.muscles.at_index(i)->orgLenPow2; // !!! POW2 !!!
        writeBuff(buff, offset, tmp2);
    }

    // linkObst
    /*
        int PhWorld::createNewLinkObst(int linkId, int collG = 0) {
    */
    len = phisics.linkObst.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.linkObst.size; ++i) {
        int tmp = phisics.linkObst.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        tmp = phisics.linkObst.at_index(i)->linkId;
        writeBuff(buff, offset, tmp);
        tmp = phisics.linkObst.at_index(i)->collisionGroup;
        writeBuff(buff, offset, tmp);
    }

    // rocketThrs
    /*
        int PhWorld::createNewThrOn(int attached, int facing, double shift_direction, double fuelConsumption = .3, double forceMult = 1) {
            * + double power
            * + int fuel_source
            * + char[8] controlls
    */
    len = phisics.rocketThrs.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.rocketThrs.size; ++i) {
        int tmp = phisics.rocketThrs.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        tmp = phisics.rocketThrs.at_index(i)->attachedPID;
        writeBuff(buff, offset, tmp);
        tmp = phisics.rocketThrs.at_index(i)->facingPID;
        writeBuff(buff, offset, tmp);

        double tmp2 = phisics.rocketThrs.at_index(i)->dirOffset;
        writeBuff(buff, offset, tmp2);

        tmp2 = phisics.rocketThrs.at_index(i)->fuelConsumption;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.rocketThrs.at_index(i)->fuelForceMulti;
        writeBuff(buff, offset, tmp2);

        tmp2 = phisics.rocketThrs.at_index(i)->power;
        writeBuff(buff, offset, tmp2);
        tmp = phisics.rocketThrs.at_index(i)->fuelContId;
        writeBuff(buff, offset, tmp);

        if (phisics.rocketThrs.at_index(i)->forPlayerID == playerID) {
            for (int j = 0; j < 8; ++j) {
                char tmp3 = phisics.rocketThrs.at_index(i)->controlls[j];
                writeBuff(buff, offset, tmp3);
            }
        } else {
            char tmp3 = '\0';
            for (int j = 0; j < 8; ++j) {
                writeBuff(buff, offset, tmp3);
            }
        }
    }

    // fuelConts
    /*
        int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
    len = phisics.fuelConts.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.fuelConts.size; ++i) {
        int tmp = phisics.fuelConts.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        double tmp2 = phisics.fuelConts.at_index(i)->capacity;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.fuelConts.at_index(i)->recharge;
        writeBuff(buff, offset, tmp2);

        for (int j = 0; j < 4; ++j) {
            tmp = phisics.fuelConts.at_index(i)->pointIDs[j];
            writeBuff(buff, offset, tmp);
        }

        tmp2 = phisics.fuelConts.at_index(i)->empty_kg;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.fuelConts.at_index(i)->kg_perUnit;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.fuelConts.at_index(i)->Ns_perUnit;
        writeBuff(buff, offset, tmp2);
    }

    // textures
    /*
        (chars, ends with \0) path
        --- FastCont<PhTextureTris>
        (int) len
        (int) idA, idB, idC
        (double) normA_x, normA_y, normB...
    */
    len = phisics.textures.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.textures.size; ++i) {
        int tmp = phisics.textures.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        for (int j = 0; j < phisics.textures.at_index(i)->orgPath.size(); ++j) {
            char c = phisics.textures.at_index(i)->orgPath[j];
            writeBuff(buff, offset, c);
        }
        char c = '\0';
        writeBuff(buff, offset, c);

        uint32_t jn = phisics.textures.at_index(i)->indiciesTrises.size;
        writeBuff(buff, offset, jn);
        for (uint32_t j = 0; j < jn; ++j) {
            PhTextureTris *tr = phisics.textures.at_index(i)->indiciesTrises.at_index(j);
            tmp = tr->idA;
            writeBuff(buff, offset, tmp);
            tmp = tr->idB;
            writeBuff(buff, offset, tmp);
            tmp = tr->idC;
            writeBuff(buff, offset, tmp);

            double tmp2 = tr->normA.x;
            writeBuff(buff, offset, tmp2);
            tmp2 = tr->normA.y;
            writeBuff(buff, offset, tmp2);

            tmp2 = tr->normB.x;
            writeBuff(buff, offset, tmp2);
            tmp2 = tr->normB.y;
            writeBuff(buff, offset, tmp2);

            tmp2 = tr->normC.x;
            writeBuff(buff, offset, tmp2);
            tmp2 = tr->normC.y;
            writeBuff(buff, offset, tmp2);
        }
    }

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        if (network_clientId == -1) {
            client.sendData(buff, offset);
#ifdef CONSOLE_LOGGING
            cout << "- init sent as client\n";
#endif
        } else {
            server.sendData(network_clientId, buff, offset);
#ifdef CONSOLE_LOGGING
            cout << "- init sent as server (length: " << offset << ")\n";
#endif
        }

        // cout << "Data sent\n";
    }
}

/*
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

// poslje del vsega
void Game::send_update_all(int clientId) {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_UPDATE_ALL;
    uint64_t offset = 2;

    // points
    /*
        int id
        double pos_x, pos_y
        double vel_x, vel_y
        * double added_weight
    */
    uint32_t len = phisics.points.size;
    writeBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id = phisics.points.get_id_at_index(i);
        writeBuff(buff, offset, id);

        Point tmp = phisics.points.at_index(i)->pos;
        writeBuff(buff, offset, tmp.x);
        writeBuff(buff, offset, tmp.y);

        tmp = phisics.points.at_index(i)->vel;
        writeBuff(buff, offset, tmp.x);
        writeBuff(buff, offset, tmp.y);

        double added_weight = phisics.points.at_index(i)->addedMass;
        writeBuff(buff, offset, added_weight);
    }

    // rocketThrs
    /*
        int id
        double power
    */
    len = phisics.rocketThrs.size;
    writeBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id = phisics.rocketThrs.get_id_at_index(i);
        writeBuff(buff, offset, id);

        double power = phisics.rocketThrs.at_index(i)->power;
        writeBuff(buff, offset, power);
    }

    // fuelConts
    /*
        int id
        double currentFuel
    */
    len = phisics.fuelConts.size;
    writeBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id = phisics.fuelConts.get_id_at_index(i);
        writeBuff(buff, offset, id);

        double currentFuel = phisics.fuelConts.at_index(i)->currentFuel;
        writeBuff(buff, offset, currentFuel);
    }

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        if (clientId == -1) {
            client.sendData(buff, offset);
#ifdef CONSOLE_LOGGING
            cout << "- all updated data sent as client\n";
#endif
        } else {
            server.sendData(clientId, buff, offset);
#ifdef CONSOLE_LOGGING
            cout << "- all updated data sent as server (length: " << offset << ")\n";
#endif
        }

        // cout << "Data sent\n";
    }
}

/*
header: DATA, CONTROLS
data: (int)count, (int)thrID_1, (double)power_1, _2, _2,...
*/

void Game::process_updatePlayerControls(RecievedData *rec) {
    uint32_t offset = 2;

    int n;
    readBuff(rec->data, offset, n);

    for (int i = 0; i < n; ++i) {
        int thrId;
        double st;

        readBuff(rec->data, offset, thrId);
        readBuff(rec->data, offset, st);

        phisics.rocketThrs.at_id(thrId)->setState(st);
    }
}