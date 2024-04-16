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
    while (g->running) {
        if (!g->networkingActive || g->client.getConnectionStatus() != 0) {
            Sleep(1000);
            continue;
        }

    cancel_new_client:
        int id = g->server.acceptNewClient();
        if (id >= 0) { // new client
            Timer t;
            t.interval();
            int res;
            do {
                Sleep(50);
                res = g->server.recieveData(id);
            } while (res == recieveData_NO_NEW_DATA && t.getTime() < 1);

            if (res == recieveData_NO_NEW_DATA) {
                cout << "client timed-out on providing login information! (1 sec)\n";
            } else {
                RecievedData *rec = g->server.getLastData(id);
                if (rec->len == 1 && rec->data[0] == 0) { // ping
                    char data[] = {1};
                    g->server.sendData(id, data, 1); // pong
                    g->server.closeConnection(id);
                    goto cancel_new_client;
                }

                int loginId = g->resolve_loginInfo(rec);
                if (loginId == -1) { // no username/password exists
                    cout << "This username/password don't exist!\n";
                    g->server.closeConnection(id);
                    // TODO back to client
                } else if (g->clientIds.at_id(loginId) != nullptr) {
                    cout << "This user is already on the server!\n";
                    g->server.closeConnection(id);
                    // TODO -||-
                } else {
                    g->halt = true;
                    g->clientIds.force_import(loginId, id);
                    cout << "Hello " << id << "!\n";
                    while (!g->halting)
                        asm("nop");

                    Point pos = login.at_id(loginId)->logoutPos; // TODO
                    g->gen.newPlayerAt(pos, id);

                    g->send_init(id, id);

                    g->halt = false;
                }
            }
        }

        for (int i = 0; i < g->clientIds.size(); ++i) {
            int id = *g->clientIds.at_index(i);
            int res = g->server.recieveData(id);
            switch (res) {
            case recieveData_NO_CLIENT_ERR:
            case recieveData_CONN_CLOSED_BY_CLIENT_ERR:
                g->halt = true;
                while (!g->halting)
                    asm("nop");

                cout << "Connection to " << id << " forcefully closed!\n";

                g->handle_playerLeft(id);
                g->clientIds.remove_index(i);
                --i;
                break;

            case recieveData_OK: {
                RecievedData *rec = g->server.getLastData(id);
                g->halt = true;
                while (!g->halting)
                    asm("nop");

                if (rec->len < 1)
                    break; // minimum header len

                if (rec->len == 1) { // ping packet
                    if (rec->data[0] != 0) break;
                    char data[] = {1};
                    g->server.sendData(id, data, 1); // pong
                    break;
                }

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
                    case NETSTD_DROP_ITEM:
                        g->process_drop(rec);
                        break;
                    case NETSTD_BUILD:
                        g->process_buildShip(rec, id);
                        break;
                    case NETSTD_SITDOWN:
                        g->process_sitdown(rec, id);
                        break;
                    case NETSTD_STANDUP:
                        g->process_standup(rec, id);
                        break;
                    default:
                        cout << "HEADER_DATA: unknown data\n";
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

                    case NETSTD_BYE:
                        cout << "Connection to " << id << " closed\n";
                        g->handle_playerLeft(id);
                        {
                            char data[2] = {NETSTD_HEADER_DATA, NETSTD_BYE};
                            g->server.sendData(id, data, 2);
                        }
                        g->clientIds.remove_index(i);
                        --i;
                        break;
                    default:
                        cout << "HEADER_REQUEST: unknown request\n";
                        break;
                    }

                    break;
                default:
                    cout << "HEADER: unknown operation\n";
                    break;
                }

                break;
            }

            case recieveData_NO_NEW_DATA:
            default:
                break;
            } // konc vseh CASE-ov

            g->halt = false;
        }

        for (int i = 0; i < g->clientIds.size(); ++i) {
            g->send_removedPoints(*g->clientIds.at_index(i));
        }
        g->removedPoints.clear(); // size = 0
        g->removedPoints.reset(); // rolling id = 0
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

/** @return -1 on unsuccessful login, or return ID of user on success (>=0) */
int Game::resolve_loginInfo(RecievedData *rec) {
    string username = "", password = "";
    uint32_t offset = 0;

    uint16_t len;
    readBuff(rec->data, offset, len);
    for (uint16_t i = 0; i < len; ++i) {
        char c;
        readBuff(rec->data, offset, c);
        username += c;
    }

    readBuff(rec->data, offset, len);
    for (uint16_t i = 0; i < len; ++i) {
        char c;
        readBuff(rec->data, offset, c);
        password += c;
    }

    cout << "usr=" << username << ", pas=" << password << endl;

    for (int i = 0; i < login.size(); ++i) {
        LoginEntry *usr = login.at_index(i);
        if (usr->username == username && usr->password == password) {
            return login.get_id_at_index(i);
        }
    }
    return -1;
}

void Game::send_removedPoints(int clientID) {
    if (removedPoints.size() == 0) return;

    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_DELETE;
    uint64_t offset = 2;

    // -------- BODY --------
    // meta
    /*
       uint32_t len
    */
    uint32_t len = removedPoints.size();
    writeBuff(buff, offset, len);

    // ids
    /*
        int id
    */
    for (uint32_t i = 0; i < len; ++i) {
        int id = *removedPoints.at_index(i);
        writeBuff(buff, offset, id);
    }

    server.sendData(clientID, buff, offset);
}

void Game::handle_playerLeft(int playerID) {
    int loginId = -1;
    for (int i = 0; i < clientIds.size(); ++i) {
        if (*clientIds.at_index(i) == playerID) {
            loginId = clientIds.get_id_at_index(i);
            break;
        }
    }

    if (loginId != -1) {
        for (int i = 0; i < phisics.points.size(); ++i) {
            if (phisics.points.at_index(i)->ownership == playerID && phisics.points.at_index(i)->virt) {
                // ce je owner in je virt tocka to
                login.at_id(loginId)->logoutPos = phisics.points.at_index(i)->pos;
                break;
            }
        }
    }

    for (int i = 0; i < phisics.points.size(); ++i) {
        int id = phisics.points.get_id_at_index(i);
        PhPoint *p = phisics.points.at_index(i);
        if (phisics.points.at_index(i)->ownership == playerID) {
            removedPoints.push_back(id);
            phisics.removePointById(id, &removedPoints); // zbrise tega, pise se v tale list kar se pobrise
            i--;
        }
    }
}

// TODO not in use anymore
void Game::handle_newPlayer(int playerID) {
    gen.newPlayerAt({(double)playerID, 0}, playerID);
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
    uint32_t len = phisics.points.size();
    writeBuff(buff, offset, len);
    cout << "points len = " << len << endl;
    for (int i = 0; i < len; ++i) {
        int tmpid = phisics.points.get_id_at_index(i);
        writeBuff(buff, offset, tmpid);

        Point tmp = phisics.points.at_index(i)->getPos();
        writeBuff(buff, offset, tmp.x);
        writeBuff(buff, offset, tmp.y);

        double tmp2 = phisics.points.at_index(i)->mass;
        writeBuff(buff, offset, tmp2);

        uint32_t jn = phisics.points.at_index(i)->collisionGroups.size();
        writeBuff(buff, offset, jn);
        for (int j = 0; j < jn; ++j) {
            int tmp3 = *phisics.points.at_index(i)->collisionGroups.at_index(j);
            writeBuff(buff, offset, tmp3);
        }

        tmp2 = phisics.points.at_index(i)->KoF_static;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.points.at_index(i)->KoF_kinetic;
        writeBuff(buff, offset, tmp2);

        bool tmp3 = phisics.points.at_index(i)->virt;
        writeBuff(buff, offset, tmp3);

        if (tmp3) {
            uint16_t len = phisics.points.at_index(i)->virtAvgPoints.size();
            writeBuff(buff, offset, len);
            for (uint16_t j = 0; j < len; ++j) {
                tmpid = *phisics.points.at_index(i)->virtAvgPoints.at_index(j);
                writeBuff(buff, offset, tmpid);
            }
        }

        tmp2 = phisics.points.at_index(i)->vel.x;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.points.at_index(i)->vel.y;
        writeBuff(buff, offset, tmp2);
    }

    // lineobst
    /*
        int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    */
    len = phisics.lineObst.size();
    writeBuff(buff, offset, len);
    cout << "lineObst len = " << len << endl;
    for (int i = 0; i < len; ++i) {
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
    len = phisics.links.size();
    writeBuff(buff, offset, len);
    cout << "links len = " << len << endl;
    for (int i = 0; i < len; ++i) {
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
    len = phisics.muscles.size();
    writeBuff(buff, offset, len);
    cout << "muscles len = " << len << endl;
    for (int i = 0; i < len; ++i) {
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
    len = phisics.linkObst.size();
    writeBuff(buff, offset, len);
    cout << "linkObst len = " << len << endl;
    for (int i = 0; i < len; ++i) {
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
    len = phisics.rocketThrs.size();
    writeBuff(buff, offset, len);
    cout << "rocketThr len = " << len << endl;
    for (uint32_t i = 0; i < len; ++i) {
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
        int ID
        bool virt
        if (virt)
            uint32_t virtPLen
            int ID1, ID2...
        else
            int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
    len = phisics.fuelConts.size();
    writeBuff(buff, offset, len);
    cout << "fuelCont len = " << len << endl;
    for (int i = 0; i < len; ++i) {
        int tmp = phisics.fuelConts.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        bool tmpVirt = phisics.fuelConts.at_index(i)->virt;
        writeBuff(buff, offset, tmpVirt);
        if (tmpVirt) {
            uint32_t virtLen = phisics.fuelConts.at_index(i)->virtIDs.size();
            writeBuff(buff, offset, virtLen);
            for (uint32_t j = 0; j < virtLen; ++j) {
                int virtID = *phisics.fuelConts.at_index(i)->virtIDs.at_index(j);
                writeBuff(buff, offset, virtID);
            }
            continue;
        }

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
    len = phisics.textures.size();
    writeBuff(buff, offset, len);
    cout << "textures len = " << len << endl;
    for (int i = 0; i < len; ++i) {
        int tmp = phisics.textures.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        for (int j = 0; j < phisics.textures.at_index(i)->orgPath.size(); ++j) {
            char c = phisics.textures.at_index(i)->orgPath[j];
            writeBuff(buff, offset, c);
        }
        char c = '\0';
        writeBuff(buff, offset, c);

        uint32_t jn = phisics.textures.at_index(i)->indiciesTrises.size();
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

    // dropped items
    /*
        (struct DroppedItem) item
    */
    len = droppedItems.size();
    writeBuff(buff, offset, len);
    cout << "dropped items len = " << len << endl;
    for (int i = 0; i < len; ++i) {
        DroppedItem tmp = *droppedItems.at_index(i);
        writeBuff(buff, offset, tmp);
    }

    // seats
    /*
        (int) atPID
    */
    len = seats.size();
    writeBuff(buff, offset, len);
    cout << "seats len = " << len << endl;
    for (int i = 0; i < len; ++i) {
        int tmp = seats.at_index(i)->PID;
        writeBuff(buff, offset, tmp);
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
    uint32_t len = phisics.points.size();
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
    len = phisics.rocketThrs.size();
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
    len = phisics.fuelConts.size();
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

        // cout << thrId << ", " << st << endl;
    }
}

// -------- inventory --------

/*
DroppedItem(struct)
*/
void Game::send_pickup(int clientId, DroppedItem it) {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_PICKUP_ITEM;
    uint64_t offset = 2;

    writeBuff(buff, offset, it);

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        server.sendData(clientId, buff, offset);
#ifdef CONSOLE_LOGGING
        cout << "- all updated data sent as server (length: " << offset << ")\n";
#endif
    }
}

void Game::process_drop(RecievedData *rec) {
    uint32_t offset = 2;

    DroppedItem it;
    readBuff(rec->data, offset, it.pos.x);
    readBuff(rec->data, offset, it.pos.y);
    readBuff(rec->data, offset, it.entr.ID);
    readBuff(rec->data, offset, it.entr.count);

    droppedItems.push_back(it);
}

void Game::process_sitdown(RecievedData *rec, int clientId) {
    uint32_t offset = 2;
    int seatID;
    readBuff(rec->data, offset, seatID);

    for (int i = 0; i < phisics.points.size(); ++i) {
        if (phisics.points.at_index(i)->ownership == clientId)
            phisics.removePointById(phisics.points.get_id_at_index(i));
    }

    send_init(clientId, -seatID);
}

void Game::process_standup(RecievedData *rec, int clientId) {
    Point pos = {};
    gen.newPlayerAt(pos, clientId);

    send_init(clientId, clientId);
}