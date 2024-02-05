#pragma once
#include "game/game.h"
string srvrName = "127.0.0.1";
Game::Game() : networkThr(networkManager, this) {
    cout << "Run as server? ";
    cin >> serverRole;

    
    running = true;
    networkingActive = false;
    phisics.gravity_accel = 0; // vesolje
    phisics.accel_mult_second = .5;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return;
    }
    wind = SDL_CreateWindow("StarCamp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0); // SDL_WINDOW_BORDERLESS namesto 0
    if (!wind) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        SDL_Quit();
        return;
    }
    cam.assignRenderer(SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!cam.r) {
        cout << "Error creating renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return;
    }

    SDL_SetRenderDrawBlendMode(cam.r, SDL_BLENDMODE_BLEND);

    TTF_Init();
    // TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Regular.ttf", 24);
    // SDL_Surface *textSurface;
    // textSurface = TTF_RenderText_Blended(Sans, "Hello world!", SDL_Color({255, 255, 255})); //use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    // SDL_Texture *textTexture = SDL_CreateTextureFromSurface(r, textSurface);

    cam.scale = 100; //  __px = 1m
    cam.x = -(cam.w / cam.scale) / 2;
    cam.y = -(cam.h / cam.scale) / 2;

    // phisics.loadWorldFromFile("TEST.WRD");

    // phisics.points.at_id(0)->collisionGroups.push_back(1);
    // phisics.points.at_id(1)->collisionGroups.push_back(1);
    // phisics.points.at_id(2)->collisionGroups.push_back(1);

    // phisics.createNewThrOn(0, 1, 20);
    // phisics.createNewThrOn(1, 2, 20);
    // phisics.createNewThrOn(2, 0, 20);

    player.init(&phisics, &kb, &cam, 0, 0);

    // particleSystem.create({1, 0}, .05, .02, PI, .5, 1, 255, 100, 0);
    // particleSystem.setSpawnInterval(.02);
    // particleSystem.setRandomises(PI/8, .01, .3);

    // -------- net --------
    client.init(srvrName);
    netRequestTimer.interval();
    send_init();
}

Game::~Game() {
    TTF_Quit();
    SDL_DestroyRenderer(cam.r);
    SDL_DestroyWindow(wind);
    SDL_Quit();

    networkThr.join();
    client.closeConnection();
}

void Game::update() {
    double dt = t.interval();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        kb.update(event);
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;

            default:
                break;
            }

            break;

        case SDL_KEYUP:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_SPACE:
                break;

            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    uint8_t m_ev = m.update();
    if (m_ev & Mouse::M_LClickMask) {
        cout << "L click at " << m.x << ", " << m.y << endl;
    }
    if (m_ev & Mouse::M_RClickMask) {
        cout << "R click at " << m.x << ", " << m.y << endl;
    }

    // if (kb.get(SDL_SCANCODE_UP)) {
    //     phisics.muscles.at_index(0)->expand();
    // }
    // if (kb.get(SDL_SCANCODE_DOWN)) {
    //     phisics.muscles.at_index(0)->contract();
    // }
    // if (kb.get(SDL_SCANCODE_LEFT)) {
    //     phisics.muscles.at_index(0)->relax();
    // }

    double dtPerStep = dt / PHISICS_SUBSTEPS;
    for (int i = 0; i < PHISICS_SUBSTEPS; ++i) {
        phisics.applyGravity();
        player.update();
        /*
        if (kb.get(SDL_SCANCODE_W)) {
            for (int i = 0; i < 3; ++i) {
                phisics.points.at_index(i)->force.y += 15 * (1 + kb.get(SDL_SCANCODE_LSHIFT) * 5);
            }
        }
        if (kb.get(SDL_SCANCODE_S)) {
            for (int i = 0; i < 3; ++i) {
                phisics.points.at_index(i)->force.y -= 15 * (1 + kb.get(SDL_SCANCODE_LSHIFT) * 5);
            }
        }
        if (kb.get(SDL_SCANCODE_A)) {
            for (int i = 0; i < 3; ++i) {
                phisics.points.at_index(i)->force.x -= 15 * (1 + kb.get(SDL_SCANCODE_LSHIFT) * 5);
            }
        }
        if (kb.get(SDL_SCANCODE_D)) {
            for (int i = 0; i < 3; ++i) {
                phisics.points.at_index(i)->force.x += 15 * (1 + kb.get(SDL_SCANCODE_LSHIFT) * 5);
            }
        }
        phisics.rocketThrs.at_id(0)->setState(kb.get(SDL_SCANCODE_UP));
        phisics.rocketThrs.at_id(1)->setState(kb.get(SDL_SCANCODE_UP));
        phisics.rocketThrs.at_id(2)->setState(kb.get(SDL_SCANCODE_UP));
        */
        phisics.update(dtPerStep);
    }

    // particleSystem.spawning = kb.get(SDL_SCANCODE_SPACE);

    for (int i = 0; i < particleSs.size; ++i)
        particleSs.at_index(i)->update(dt);

    SDL_SetRenderDrawColor(cam.r, 5, 5, 5, 255); // r b g a
    SDL_RenderClear(cam.r);

    phisics.render(&cam);
    player.render(&cam);

    for (int i = 0; i < particleSs.size; ++i)
        particleSs.at_index(i)->render(&cam);

    if (drawRuller) {
        for (uint16_t y = 0, y2 = 0; y < cam.h; y += cam.scale) {
            if (y2) {
                SDL_SetRenderDrawColor(cam.r, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(cam.r, 0, 0, 255, 255);
            }
            y2 = !y2;
            SDL_RenderDrawLine(cam.r, 0, y, 0, y + cam.scale);
        }
    }

    SDL_RenderPresent(cam.r);
}

// -------- NET --------
#include "netagent/netstds.cpp"

void Game::requestInitialFromServer() {
    if (client.getConnectionStatus() != 0) {
        cout << "E@ Game::requestInitialFromServer() - Server error!\n";
    }
    char data[] = {NETSTD_HEADER_REQUEST, NETSTD_INIT};
    client.sendData(data, sizeof(data));
}

void Game::networkManager(Game *g) {
    while (g->running) {
        if (!g->networkingActive || g->client.getConnectionStatus() != 0) {
            Sleep(1000);
            continue;
        }
        int ret = g->client.recieveData();
        if (ret != recieveData_NO_NEW_DATA) {
            cout << "Data recieved!\n";
            if (g->client.recvbuflen < 3) continue;

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
            }
        }

        if (g->netRequestTimer.getTime() >= NETW_REQ_INTERVAL) {
            g->netRequestTimer.interval();
            cout << "Time to send data\n";
            // TODO send data + request new data
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

#define writeBuff(buff, offset, a)        \
    memcpy(&buff[offset], &a, sizeof(a)); \
    offset += sizeof(a);

void Game::process_init() {
    int bufflen = client.recvbuflen;
    char buff[bufflen];
    memcpy(&buff, client.recvbuf, bufflen);

    uint64_t offset = 2;
    phisics.resetWorld();

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt
    */
    uint32_t len;
    readBuff(buff, offset, len);
    for (uint32_t i = 0; i < len; ++i) {
        int id;
        double x;
        double y;
        double mass;
        FastCont<int> collisionGroup;
        double static_koef;
        double kinetic_koef;
        bool virt;

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

        phisics.createNewLinkBetween(idA, idB, spring_koef, damp_koef, maxCompression, maxStretch, originalLength, id);
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
    cout << "Init data processed\n";
}

// prepise del vsega
void Game::process_update_all() {
}

// poslje vse
// TODO test
void Game::send_init() {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_INIT;
    uint64_t offset = 2;

    // -------- BODY --------

    // meta
    /*
        double gravity_accel;
        double accel_mult_second;
    */

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
        * + bool virt
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

    if (offset >= MAX_BUF_LEN) {
        cout << "Data buffer overflowed, not sending anything\n";
        // TODO kaj ce OF
    } else {
        client.sendData(buff, offset);
        cout << "Data sent\n";
    }
}

// poslje del vsega
void Game::send_update_all() {
}