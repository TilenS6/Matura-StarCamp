#pragma once
#include "game/game.h"
string srvrName = "127.0.0.1";

uint16_t charToScancode(char c) {
    if (c > 'Z')
        c -= 'a' - 'A';

    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + SDL_SCANCODE_A;
    } else if (c >= '1' && c <= '9') {
        return c - '1' + SDL_SCANCODE_1;
    } else if (c == '0') {
        return SDL_SCANCODE_0;
    }

    return SDL_SCANCODE_UNKNOWN;
}

Game::Game() {
    cout << "Run as server? ";
    cin >> serverRole;

    running = true;
    networkingActive = false;
    phisics.gravity_accel = 0; // vesolje
    phisics.vel_mult_second = .5;

    gameArea.a = {0, 0};
    gameArea.setRadius(20);

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
    cam.assignRenderer(SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE));
    if (!cam.r) {
        cout << "Error creating renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return;
    }

    SDL_SetRenderDrawBlendMode(cam.r, SDL_BLENDMODE_BLEND);
    cout << "- " << SDL_GetError() << endl;

    TTF_Init();
    // TTF_Font *Sans = TTF_OpenFont("fonts/open-sans/OpenSans-Regular.ttf", 24);
    // SDL_Surface *textSurface;
    // textSurface = TTF_RenderText_Blended(Sans, "Hello world!", SDL_Color({255, 255, 255})); //use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    // SDL_Texture *textTexture = SDL_CreateTextureFromSurface(r, textSurface);

    if (serverRole)
        cam.scale = 8; //  __ px = 1m
    else
        cam.scale = 50; //  __ px = 1m

    cam.x = -(cam.w / cam.scale) / 2;
    cam.y = -(cam.h / cam.scale) / 2;

    // phisics.loadWorldFromFile("TEST.WRD");

    // phisics.points.at_id(0)->collisionGroups.push_back(1);
    // phisics.points.at_id(1)->collisionGroups.push_back(1);
    // phisics.points.at_id(2)->collisionGroups.push_back(1);

    // phisics.createNewThrOn(0, 1, 20);
    // phisics.createNewThrOn(1, 2, 20);
    // phisics.createNewThrOn(2, 0, 20);

    gen.init(this);

    // particleSystem.create({1, 0}, .05, .02, PI, .5, 1, 255, 100, 0);
    // particleSystem.setSpawnInterval(.02);
    // particleSystem.setRandomises(PI/8, .01, .3);

    // -------- net --------

    if (serverRole) {
        server.init();
        networkThr = thread(networkManagerS, this);
        gen.planets(1234, 10); // seed, count
    } else {
        client.init(srvrName);
        networkThr = thread(networkManagerC, this);
        // send_init();
    }
    cout << "init completed\n";
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
    while (halt) {
        halting = true;
    }
    halting = false;

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

    if (serverRole) {
        bool faster = kb.get(SDL_SCANCODE_SPACE);
        if (kb.get(SDL_SCANCODE_W)) {
            cam.y += ((500 + faster * 500) / cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_S)) {
            cam.y -= ((500 + faster * 500) / cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_D)) {
            cam.x += ((500 + faster * 500) / cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_A)) {
            cam.x -= ((500 + faster * 500) / cam.scale) * dt;
        }

        if (kb.get(SDL_SCANCODE_LSHIFT)) {
            cam.x += (cam.w / cam.scale) / 2;
            cam.y += (cam.h / cam.scale) / 2;

            cam.scale *= 1 + dt * (1 + faster);

            cam.x -= (cam.w / cam.scale) / 2;
            cam.y -= (cam.h / cam.scale) / 2;
        }
        if (kb.get(SDL_SCANCODE_LCTRL)) {
            cam.x += (cam.w / cam.scale) / 2;
            cam.y += (cam.h / cam.scale) / 2;

            cam.scale *= 1 - dt * (1 + faster);

            cam.x -= (cam.w / cam.scale) / 2;
            cam.y -= (cam.h / cam.scale) / 2;
        }
    }

    double dtPerStep = dt / PHISICS_SUBSTEPS;
    if (dtPerStep > MAX_DT) {
        cout << "dt capped\n";
        dtPerStep = MAX_DT;
    }
    // clear send buffer
    while (thrSendBuffer.size > 0)
        thrSendBuffer.pop_back();

    for (int i = 0; i < PHISICS_SUBSTEPS; ++i) {
        phisics.applyGravity();

        // -------- CLINET
        if (!serverRole) {
            for (int i = 0; i < phisics.rocketThrs.size; ++i) {
                if (phisics.rocketThrs.at_index(i)->controlls[0] == '\0') continue;

                double st = 0;
                for (int j = 0; j < 8; ++j) {
                    char c = phisics.rocketThrs.at_index(i)->controlls[j];
                    if (c == '\0')
                        break;

                    if (kb.get((SDL_Scancode)charToScancode(c))) {
                        st = 1;
                        // cout << "PRTISNU " << c << endl;
                        break;
                    }
                }
                if (phisics.rocketThrs.at_index(i)->power != st) {
                    thrSendBuffer.force_import(phisics.rocketThrs.get_id_at_index(i), st);
                    phisics.rocketThrs.at_index(i)->setState(st);
                }
            }

            if (thrSendBuffer.size > 0)
                send_updatePlayerControls();
        }
        // -------- END CLIENT

        phisics.update(dtPerStep);

        for (int i = 0; i < particleSs.size; ++i)
            particleSs.at_index(i)->update(dt);
    } // end: substeping

    if (!serverRole)
        followCamera(dt);

    render();
}

void Game::followCamera(double dt) {
    FastCont<Point> controllsAt;
    Point avg = {0, 0}, min = {INFINITY, INFINITY}, max = {-INFINITY, -INFINITY};
    int count = 0;
    for (int i = 0; i < phisics.rocketThrs.size; ++i) {
        if (phisics.rocketThrs.at_index(i)->controlls[0] != '\0') {
            ++count;
            int pid = phisics.rocketThrs.at_index(i)->attachedPID;
            Point a = phisics.points.at_id(pid)->pos;
            avg += a;
            if (min.x > a.x) min.x = a.x;
            if (max.x < a.x) max.x = a.x;
            if (min.y > a.y) min.y = a.y;
            if (max.y < a.y) max.y = a.y;
        }
    }
    if (count == 0) {
        cam.x = 0;
        cam.y = 0;
        return;
    }
    
    avg /= (double)count;

    Point newPos = {avg.x - (cam.w / cam.scale) / 2, avg.y - (cam.h / cam.scale) / 2};
    double k = pow(CAMERA_STIFFNESS, dt);
    cam.x = cam.x * k + newPos.x * (1. - k);
    cam.y = cam.y * k + newPos.y * (1. - k);

    if (isnan(cam.x) || isnan(cam.y)) {
        cam.x = newPos.x;
        cam.y = newPos.y;
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(cam.r, 5, 5, 5, 255); // r b g a
    SDL_RenderClear(cam.r);

    // TODO zvezde

    // planeti
    for (int i = 0; i < planets.size; ++i) {
        planets.at_index(i)->render(&cam);
    }

    phisics.render(&cam);

    // if (serverRole)
    //    player.render(&cam);

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

    Point p = {0, 0};
    SDL_SetRenderDrawColor(cam.r, 255, 0, 0, 255); // r b g a
    p.render(&cam);

    class Rectangle rec;
    SDL_SetRenderDrawColor(cam.r, 255, 255, 255, 255); // r b g a
    rec.a = {0, 0};
    rec.dimensions = {1, 1};
    rec.render(&cam);

    SDL_RenderPresent(cam.r);
}

// -------- NET --------
#include "netagent/netstds.cpp"

#include "game/game_networking_c.cpp"
#include "game/game_networking_s.cpp"