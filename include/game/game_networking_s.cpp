#include "game/game.h"
using namespace std;

void Game::networkManagerS(Game *g)
{
    cout << "Server ran...\n";
    FastCont<int> clientIds;
    while (g->running)
    {
        if (!g->networkingActive || g->client.getConnectionStatus() != 0)
        {
            Sleep(1000);
            continue;
        }

        int id = g->server.acceptNewClient();
        if (id >= 0)
        { // new client
            clientIds.push_back(id);
            cout << "Hello " << id << "!\n";
            g->send_init(id);
        }

        for (int i = 0; i < clientIds.size; ++i)
        {
            int id = *clientIds.at_index(i);
            int res = g->server.recieveData(id);
            switch (res)
            {
            case recieveData_NO_CLIENT_ERR:
            case recieveData_CONN_CLOSED_BY_CLIENT_ERR:
                clientIds.remove_index(i);
                --i;
                break;

            case recieveData_OK:
            {
                RecievedData *rec = g->server.getLastData(id);
                if (rec->len < 2)
                    break; // minimum header len

                // char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};

                switch (rec->data[0])
                {
                case NETSTD_HEADER_DATA:
                    //! INCOMING DATA
                    switch (rec->data[1])
                    {
                    case NETSTD_UPDATE_PLAYER_CONTROLS:
                        g->process_updatePlayerControls();
                        break;
                    default:
                        break;
                    }
                    break;
                case NETSTD_HEADER_REQUEST:
                    //! REQUEST

                    switch (rec->data[1])
                    {
                    case NETSTD_INIT:
                        cout << "- init requested\n";
                        g->send_init(id);
                        break;
                    case NETSTD_UPDATE_ALL:
                        cout << "- update all requested\n";
                        g->send_update_all(id);
                        break;
                    default:
                        break;
                    }

                    break;
                default:
                    break;
                }

                break;
            }

            case recieveData_NO_NEW_DATA:
            default:
                break;
            }
        }
    }
}

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

// poslje vse
// TODO test
void Game::send_init(int clientId)
{
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
    */
    writeBuff(buff, offset, phisics.gravity_accel);
    writeBuff(buff, offset, phisics.vel_mult_second);

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt
        * + double velocity_x, double velocity_y
    */
    uint32_t len = phisics.points.size;
    memcpy(&buff[offset], &len, sizeof(len));
    offset += sizeof(len);
    for (int i = 0; i < phisics.points.size; ++i)
    {
        int tmpid = phisics.points.get_id_at_index(i);
        writeBuff(buff, offset, tmpid);

        Point tmp = phisics.points.at_index(i)->getPos();
        writeBuff(buff, offset, tmp.x);
        writeBuff(buff, offset, tmp.y);

        double tmp2 = phisics.points.at_index(i)->mass;
        writeBuff(buff, offset, tmp2);

        uint32_t jn = phisics.points.at_index(i)->collisionGroups.size;
        writeBuff(buff, offset, jn);
        for (int j = 0; j < jn; ++j)
        {
            int tmp3 = *phisics.points.at_index(i)->collisionGroups.at_index(j);
            writeBuff(buff, offset, tmp3);
        }

        tmp2 = phisics.points.at_id(i)->KoF_static;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.points.at_id(i)->KoF_kinetic;
        writeBuff(buff, offset, tmp2);

        bool tmp3 = phisics.points.at_id(i)->virt;
        writeBuff(buff, offset, tmp3);

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
    for (int i = 0; i < phisics.lineObst.size; ++i)
    {
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
    for (int i = 0; i < phisics.links.size; ++i)
    {
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
    for (int i = 0; i < phisics.muscles.size; ++i)
    {
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
    for (int i = 0; i < phisics.linkObst.size; ++i)
    {
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
    for (int i = 0; i < phisics.rocketThrs.size; ++i)
    {
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

        for (int j = 0; j < 8; ++j)
        {
            char tmp3 = phisics.rocketThrs.at_index(i)->controlls[j];
            writeBuff(buff, offset, tmp3);
        }
    }

    // fuelConts
    /*
        int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
    len = phisics.fuelConts.size;
    writeBuff(buff, offset, len);
    for (int i = 0; i < phisics.fuelConts.size; ++i)
    {
        int tmp = phisics.fuelConts.get_id_at_index(i);
        writeBuff(buff, offset, tmp);

        double tmp2 = phisics.fuelConts.at_index(i)->capacity;
        writeBuff(buff, offset, tmp2);
        tmp2 = phisics.fuelConts.at_index(i)->recharge;
        writeBuff(buff, offset, tmp2);

        for (int j = 0; j < 4; ++j)
        {
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

    if (offset >= MAX_BUF_LEN)
    {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    }
    else
    {
        if (clientId == -1)
        {
            client.sendData(buff, offset);
            cout << "- init sent as client\n";
        }
        else
        {
            server.sendData(clientId, buff, offset);
            cout << "- init sent as server (length: " << offset << ")\n";
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
void Game::send_update_all(int clientId)
{
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
    for (uint32_t i = 0; i < len; ++i)
    {
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
    for (uint32_t i = 0; i < len; ++i)
    {
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
    for (uint32_t i = 0; i < len; ++i)
    {
        int id = phisics.fuelConts.get_id_at_index(i);
        writeBuff(buff, offset, id);

        double currentFuel = phisics.fuelConts.at_index(i)->currentFuel;
        writeBuff(buff, offset, currentFuel);
    }

    if (offset >= MAX_BUF_LEN)
    {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    }
    else
    {
        if (clientId == -1)
        {
            client.sendData(buff, offset);
            cout << "- all updated data sent as client\n";
        }
        else
        {
            server.sendData(clientId, buff, offset);
            cout << "- all updated data sent as server (length: " << offset << ")\n";
        }

        // cout << "Data sent\n";
    }
}

void Game::process_updatePlayerControls()
{
    // TODO
    cout << "todo process_updatePlayerControls()\n";
}