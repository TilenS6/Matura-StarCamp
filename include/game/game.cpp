#pragma once
#include "game/game.h"
string srvrName = "127.0.0.1";
Game::Game() : networkThr(networkManager, this) {
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

    send_init();
    // particleSystem.create({1, 0}, .05, .02, PI, .5, 1, 255, 100, 0);
    // particleSystem.setSpawnInterval(.02);
    // particleSystem.setRandomises(PI/8, .01, .3);

    // -------- net --------
    client.init(srvrName);
    netRequestTimer.interval();
    networkingActive = true;
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
initial (cel world, vse):
    FastCont<PhPoint> points; (id )
    FastCont<PhLineObst> lineObst;
    FastCont<PhLink> links;
    FastCont<PhMuscle> muscles;
    FastCont<PhLinkObst> linkObst;
    FastCont<PhRocketThr> rocketThrs;
    FastCont<PhWeight> weights;
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
void Game::process_init() {
}

// prepise del vsega
void Game::process_update_all() {
}

// poslje vse
void Game::send_init() {
    char buff[MAX_BUF_LEN];
    // header
    buff[0] = NETSTD_HEADER_DATA;
    buff[1] = NETSTD_INIT;
    uint32_t offset = 2;

    // -------- BODY --------

    // meta
    /*
        double gravity_accel;
        double accel_mult_second;
    */

    // points
    /*
        int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
    */
    // TODO len

    uint32_t len = phisics.points.size;
    memcpy(&buff[offset], &len, sizeof(len));
    offset += sizeof(len);
    for (int i = 0; i < phisics.points.size; ++i) {
        int tmpid = phisics.points.get_id_at_index(i);
        memcpy(&buff[offset], &tmpid, sizeof(tmpid));
        offset += sizeof(tmpid);

        Point tmp = phisics.points.at_index(i)->getPos();
        memcpy(&buff[offset], &tmp.x, sizeof(tmp.x));
        offset += sizeof(tmp.x);
        memcpy(&buff[offset], &tmp.y, sizeof(tmp.y));
        offset += sizeof(tmp.y);

        double tmp2 = phisics.points.at_index(i)->mass;
        memcpy(&buff[offset], &tmp2, sizeof(tmp2));
        offset += sizeof(tmp2);

        uint32_t jn = phisics.points.at_index(i)->collisionGroups.size;
        memcpy(&buff[offset], &jn, sizeof(jn));
        offset += sizeof(jn);
        for (int j = 0; j < jn; ++j) {
            int tmp3 = *phisics.points.at_index(i)->collisionGroups.at_index(j);
            memcpy(&buff[offset], &tmp3, sizeof(tmp3));
            offset += sizeof(tmp3);
        }

        tmp2 = phisics.points.at_id(i)->KoF_static;
        memcpy(&buff[offset], &tmp2, sizeof(tmp2));
        offset += sizeof(tmp2);
        tmp2 = phisics.points.at_id(i)->KoF_kinetic;
        memcpy(&buff[offset], &tmp2, sizeof(tmp2));
        offset += sizeof(tmp2);
    }

    // lineobst
    /*
        int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    */
    // TODO len
    for (int i = 0; i < phisics.lineObst.size; ++i) {
        Line tmp = phisics.lineObst.at_index(i)->line;
        memcpy(&buff[offset], &tmp.a.x, sizeof(tmp.a.x));
        offset += sizeof(tmp.a.x);
        memcpy(&buff[offset], &tmp.a.y, sizeof(tmp.a.y));
        offset += sizeof(tmp.a.y);

        memcpy(&buff[offset], &tmp.b.x, sizeof(tmp.b.x));
        offset += sizeof(tmp.b.x);
        memcpy(&buff[offset], &tmp.b.y, sizeof(tmp.b.y));
        offset += sizeof(tmp.b.y);

        int tmp2 = phisics.lineObst.at_index(i)->collisionGroup;
        memcpy(&buff[offset], &tmp2, sizeof(tmp2));
        offset += sizeof(tmp2);
    }

    // links
    /*
        int PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */

    // muscles
    /*
        int PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    */

    // linkObst
    /*
        int PhWorld::createNewLinkObst(int linkId, int collG = 0) {
    */

    // rocketThrs
    /*
        int PhWorld::createNewThrOn(int attached, int facing, double thrust, double shift_direction, double fuelConsumption = .3, double forceMult = 1) {
    */

    // weights
    /*
        int PhWorld::createNewWeightOn(int for_point_id) {
    */

    // fuelConts
    /*
        int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit=50000) {
    */
}

// poslje del vsega
void Game::send_update_all() {
}