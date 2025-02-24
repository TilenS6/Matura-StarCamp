#include "game/game.h"
#pragma once
string srvrName = "127.0.0.1";

uint16_t charToScancode(char c) {
    if (c >= 'a')
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

/*
void testF() {
    cout << "sm v f.\n";
}
*/

Game::Game(GameRenderer *_grend, string srvr, string username, string password, bool launchAsServer) {
    // global_game = this;
    grend = _grend;
    serverRole = launchAsServer;

    running = true;
    halt = false;
    halting = false;
    networkingActive = false;
    phisics.gravity_accel = 0; // vesolje
    phisics.vel_mult_second = .5;

    quitInfo = "";

    gameArea.a = {0, 0};
    gameArea.setRadius(200);

    if (serverRole)
        grend->cam.scale = 8; //  __ px = 1m
    else
        grend->cam.scale = 50; //  __ px = 1m

    grend->cam.x = -(grend->cam.w / grend->cam.scale) / 2;
    grend->cam.y = -(grend->cam.h / grend->cam.scale) / 2;

    client_inventory.loadTextures(&_grend->cam);

    gen.init(this);

    gen.stars(300);

    // -- ship builder
#ifndef PRESENTATION_MODE
    shipbuilder.init(2, 0, this);

    InteractiveButton buildBtn;
    int btnID = intButtons.push_back(buildBtn);
    intButtons.at_id(btnID)->init({-1., 0.}, "Build", &grend->cam, onpress_build);

    btnID = intButtons.push_back(buildBtn);
    intButtons.at_id(btnID)->init({4., -4.}, "Demo", &grend->cam, onpress_demo);

    // -- ore processor
    OreProcessor orepr;
    orepr.setRect(2, -4, 1, 1);
    oreProcessors.push_back(orepr);
#endif

    sitted = false;

    playermenu.init(grend->cam.r, &m);

    // -------------- TEST --------------
    /*
    InteractiveDropoffArea tmp;
    tmp.setRect(0, -2, 2, 2);
    dropoffAreas.push_back(tmp);
    */

    /*
    InteractiveButton tmp2;
    tmp2.init({-1., 0.}, "Test", &grend->cam, testF);
    intButtons.push_back(tmp2);
    */

#ifndef PRESENTATION_MODE
    EnemyTurret tmp;
    int tmpid = enemyTurrets.push_back(tmp);
    enemyTurrets.at_id(tmpid)->init({-20, 10}, 5);
#endif
    // -------------- /TEST --------------

    // -------- net --------

    if (serverRole) {
        LoginEntry entr = {"a", "a", {1, 1}};
        login.push_back(entr);
        entr = {"b", "b", {-1, 1}};
        login.push_back(entr);
        entr = {"c", "c", {-2, 1}};
        login.push_back(entr);
        entr = {"d", "d", {-3, 1}};
        login.push_back(entr);

        server.init();
        networkThr = thread(networkManagerS, this);
        gen.planets(1234, 20); // seed, count
#ifndef PRESENTATION_MODE
        gen.asteroids2(5, gameArea.getRadius() / 4, {0, 0});
#endif
    } else {
        client.init(srvr);
        send_loginInfo(username, password);
        networkThr = thread(networkManagerC, this);
        // send_init();
    }
#ifdef CONSOLE_LOGGING_STAGES
    cout << "init completed\n";
#endif
    networkingActive = true;
}

Game::~Game() {
    // halting = true;
    running = false;
    if (networkThr.joinable())
        networkThr.join();
    if (!serverRole && client.getConnectionStatus() != 1)
        client.closeConnection();
}
void Game::end() {
    // if (networkThr.joinable())
    //     networkThr.join();
    if (!serverRole && client.getConnectionStatus() == 0)
        client.closeConnection();
}

void Game::update() {
    while (halt) {
        halting = true;
    }
    halting = false;

    kb.newFrame();
    double dt = t.interval();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        kb.update(event);
        switch (event.type) {
        case SDL_QUIT:
            send_myInventory();
            running = false;
            break;

        case SDL_KEYDOWN:

        { // ZA: client_inventory
            char c = SDL_GetKeyName(event.key.keysym.sym)[0];
            if (c >= '1' && c <= '9') {
                client_inventory.selected = c - '1';
                if (client_inventory.selected < 0) client_inventory.selected = 0;
                if (client_inventory.selected >= INVENTORY_SIZE) client_inventory.selected = INVENTORY_SIZE - 1;
            }
        }
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                if (playermenu.getState()) {
                    playermenu.display(false);
                    break;
                }
                send_myInventory();
                running = false;
                break;
            case SDL_SCANCODE_B: // drop from selected inventory slot
                if (!serverRole) {
                    Point p = {m.x / grend->cam.scale + grend->cam.x, (grend->cam.h - m.y) / grend->cam.scale + grend->cam.y};
                    int sel = client_inventory.selected;
                    if (client_inventory.inv[sel].ID == none) break;
                    int dropAmount = 1; // client_inventory.inv[sel].count
                    int ret = dropInventoryItem(sel, dropAmount, p);
#ifdef CONSOLE_LOGGING_STAGES
                    cout << "drop: " << ret << endl;
#endif
                }
                break;

            case SDL_SCANCODE_RCTRL: // request physics data refresh
                request_initialFromServer();
                break;

            case SDL_SCANCODE_TAB: // toggle player menu
                playermenu.toggle();
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
        case SDL_MOUSEWHEEL:
            if (!serverRole) {
                client_inventory.selected -= event.wheel.y;
                if (client_inventory.selected < 0) client_inventory.selected = 0;
                if (client_inventory.selected >= INVENTORY_SIZE) client_inventory.selected = INVENTORY_SIZE - 1;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                // cout << "resizan na " << event.window.data1 << ", " << event.window.data2 << endl;
                grend->cam.w = event.window.data1;
                grend->cam.h = event.window.data2;
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
        cout << "R click at (world) " << m.x / grend->cam.scale + grend->cam.x << ", " << (grend->cam.h - m.y) / grend->cam.scale + grend->cam.y << endl;
    }

    // kb hijacking --
    for (int i = 0; i < dropoffAreas.size(); ++i) {
        dropoffAreas.at_index(i)->updateHijack(&kb, &m, &client_inventory, &grend->cam);
    }
    shipbuilder.updateHijack(&kb, &m, &client_inventory, &grend->cam);

    // -- kb hijacking
    for (int i = 0; i < intButtons.size(); ++i) {
        switch (intButtons.at_index(i)->update(playerMedian, dt, &kb)) {
        case onpress_notpressed:
            break;

        case onpress_build:
            shipbuilder.build();
            break;
        case onpress_sit:
            // send_sitdown(intButtons.at_index(i)->moreData);
            break;
        case onpress_demo:
            request_demoInv();

            break;
        default:
            cout << "button: unknown operation!\n";
        }
    }
    if (sitted) {
        for (int i = 0; i < seats.size(); ++i)
            seats.at_index(i)->update(dt);

        if (kb.pressedNow(SDL_SCANCODE_F)) {
            send_standup();
            sitted = false;
        }
    } else {
        for (int i = 0; i < seats.size(); ++i) {
            bool pr = seats.at_index(i)->update(dt);
            if (pr) {
                send_sitdown(seats.get_id_at_index(i));
                sitted = true;
                break;
            }
        }
    }

    if (serverRole) {
        bool faster = kb.get(SDL_SCANCODE_SPACE);
        if (kb.get(SDL_SCANCODE_W)) {
            grend->cam.y += ((500 + faster * 500) / grend->cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_S)) {
            grend->cam.y -= ((500 + faster * 500) / grend->cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_D)) {
            grend->cam.x += ((500 + faster * 500) / grend->cam.scale) * dt;
        }
        if (kb.get(SDL_SCANCODE_A)) {
            grend->cam.x -= ((500 + faster * 500) / grend->cam.scale) * dt;
        }

        if (kb.get(SDL_SCANCODE_LSHIFT)) {
            grend->cam.x += (grend->cam.w / grend->cam.scale) / 2;
            grend->cam.y += (grend->cam.h / grend->cam.scale) / 2;

            grend->cam.scale *= 1 + dt * (1 + faster);

            grend->cam.x -= (grend->cam.w / grend->cam.scale) / 2;
            grend->cam.y -= (grend->cam.h / grend->cam.scale) / 2;
        }
        if (kb.get(SDL_SCANCODE_LCTRL)) {
            grend->cam.x += (grend->cam.w / grend->cam.scale) / 2;
            grend->cam.y += (grend->cam.h / grend->cam.scale) / 2;

            grend->cam.scale *= 1 - dt * (1 + faster);

            grend->cam.x -= (grend->cam.w / grend->cam.scale) / 2;
            grend->cam.y -= (grend->cam.h / grend->cam.scale) / 2;
        }
    }

    double dtPerStep = dt / PHISICS_SUBSTEPS;
    if (dtPerStep > MAX_DT) {
#ifdef CONSOLE_LOGGING_DT_CAPPED
        cout << "dt capped\n";
#endif
        dtPerStep = MAX_DT;
    }
    // clear send buffer
    thrSendBuffer.clear();
    thrSendBuffer.reset();

    // update player rotation:
    for (int i = 0; i < phisics.points.size(); ++i) {
        if (phisics.points.at_index(i)->ownership == myPlayerID && phisics.points.at_index(i)->virtAvgPoints.size() == 2) {
            playerHeadPos = phisics.points.at_index(i)->getPos();
            double dx = playerHeadPos.x - playerMedian.x;
            double dy = playerHeadPos.y - playerMedian.y;
            playerRotation = atan2(dy, dx);
            break;
        }
    }

    shotTimer += dt;
    if (shotTimer >= SHOTCOOLDOWN * 1.1) shotTimer = SHOTCOOLDOWN;
    if (!sitted && kb.get(SDL_SCANCODE_X) && shotTimer >= SHOTCOOLDOWN) {
        shotTimer -= SHOTCOOLDOWN;

        double shotOffset = .2;
        double shotSpeed = 5;
        double damage = .55;

        double s = sin(playerRotation), c = cos(playerRotation);
        double x = playerHeadPos.x + shotOffset * c;
        double y = playerHeadPos.y + shotOffset * s;
        double sx = shotSpeed * c;
        double sy = shotSpeed * s;
        send_newProjectile(x, y, sx, sy, damage, myPlayerID);
    }

    for (int i = 0; i < PHISICS_SUBSTEPS; ++i) {
        phisics.applyGravity();

        // CLIENT --
        if (!serverRole) {
            for (int i = 0; i < phisics.rocketThrs.size(); ++i) {
                if (phisics.rocketThrs.at_index(i)->controlls[0] == '\0')
                    continue;

                double st = 0;
                for (int j = 0; j < 8; ++j) {
                    char c = phisics.rocketThrs.at_index(i)->controlls[j];
                    if (c == '\0')
                        break;

                    if (kb.get((SDL_Scancode)charToScancode(c))) {
                        st = 1;
                        break;
                    }
                }
                if (phisics.rocketThrs.at_index(i)->power != st) {
                    thrSendBuffer.force_import(phisics.rocketThrs.get_id_at_index(i), st);
                    phisics.rocketThrs.at_index(i)->setState(st);
                }
            }
        }
        // -- CLIENT

        for (int i = 0; i < projectiles.size(); ++i) {
            int removedLink = -1;
            bool selfDel = projectiles.at_index(i)->update(dtPerStep, this, &removedLink);
            if (selfDel) {
                if (removedLink != -1) {
                    removedLinks.push_back(removedLink);
                }
                int pid = projectiles.get_id_at_index(i);
                removedProjectiles.push_back(pid);
                projectiles.remove_index(i);
                --i;
            }
        }

        for (int i = 0; i < enemyTurrets.size(); ++i) {
            enemyTurrets.at_index(i)->update(dtPerStep, this);
        }

        phisics.update(dtPerStep, &removedPoints, &removedLinks);

        for (int i = 0; i < particleSs.size(); ++i)
            particleSs.at_index(i)->update(dtPerStep);
    } // end: substeping

    playermenu.update(this, dt);

    if (thrSendBuffer.size() > 0)
        send_updatePlayerControls();

    if (!serverRole) {
        followCamera(dt);
        updateInteractiveItems();
    } else {
        updatePlayersPickupFromFloor();
    }

    render();
}

void Game::followCamera(double dt) {
    Point avg = {0, 0}, min = {INFINITY, INFINITY}, max = {-INFINITY, -INFINITY};
    int count = 0;
    for (int i = 0; i < phisics.rocketThrs.size(); ++i) {
        if (phisics.rocketThrs.at_index(i)->controlls[0] != '\0') {
            ++count;
            int pid = phisics.rocketThrs.at_index(i)->attachedPID;
            Point a = phisics.points.at_id(pid)->pos;
            avg += a;
            if (min.x > a.x)
                min.x = a.x;
            if (max.x < a.x)
                max.x = a.x;
            if (min.y > a.y)
                min.y = a.y;
            if (max.y < a.y)
                max.y = a.y;
        }
    }
    if (count == 0) {
        grend->cam.x = 0;
        grend->cam.y = 0;
        return;
    }

    // avg /= (double)count;
    // mediana
    playerMedian.x = (min.x + max.x) / 2.;
    playerMedian.y = (min.y + max.y) / 2.;

    Point newPos = {playerMedian.x - (grend->cam.w / grend->cam.scale) / 2, playerMedian.y - (grend->cam.h / grend->cam.scale) / 2};
    double k = pow(CAMERA_STIFFNESS, dt);
    grend->cam.x = grend->cam.x * k + newPos.x * (1. - k);
    grend->cam.y = grend->cam.y * k + newPos.y * (1. - k);

    if (isnan(grend->cam.x) || isnan(grend->cam.y)) {
        grend->cam.x = newPos.x;
        grend->cam.y = newPos.y;
    }
}

void Game::render() {
    grend->clear();

    // zvezde
    for (int i = 0; i < stars.size(); ++i) {
        stars.at_index(i)->render(&grend->cam);
    }
    // planeti
    for (int i = 0; i < planets.size(); ++i) {
        planets.at_index(i)->render(&grend->cam);
    }

    phisics.render(&grend->cam);

    // interactive items --
    shipbuilder.render(&grend->cam);
    for (int i = 0; i < dropoffAreas.size(); ++i) {
        dropoffAreas.at_index(i)->render(&grend->cam);
    }
    for (int i = 0; i < intButtons.size(); ++i) {
        intButtons.at_index(i)->render(&grend->cam);
    }
    for (int i = 0; i < oreProcessors.size(); ++i) {
        oreProcessors.at_index(i)->render(&grend->cam);
    }
    for (int i = 0; i < seats.size(); ++i) {
        seats.at_index(i)->render();
    }
    // -- interactive items

    renderDroppedItems(&grend->cam);

    // projectiles
    for (int i = 0; i < projectiles.size(); ++i) {
        projectiles.at_index(i)->render(&grend->cam);
    }
    // turrets
#ifndef PRESENTATION_MODE
    for (int i = 0; i < enemyTurrets.size(); ++i) {
        enemyTurrets.at_index(i)->render(&grend->cam);
    }
#endif

    for (int i = 0; i < particleSs.size(); ++i)
        particleSs.at_index(i)->render(&grend->cam);

    if (drawRuller) {
        for (uint16_t y = 0, y2 = 0; y < grend->cam.h; y += grend->cam.scale) {
            if (y2) {
                SDL_SetRenderDrawColor(grend->cam.r, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(grend->cam.r, 0, 0, 255, 255);
            }
            y2 = !y2;
            SDL_RenderDrawLine(grend->cam.r, 0, y, 0, y + grend->cam.scale);
        }
    }

    // test ---
    /*
    Point p = {0, 0};
    SDL_SetRenderDrawColor(grend->cam.r, 255, 0, 0, 255); // r b g a
    p.render(&grend->cam);

    class Rectng rec;
    SDL_SetRenderDrawColor(grend->cam.r, 255, 255, 255, 255); // r b g a
    rec.a = {0, 0};
    rec.dimensions = {1, 1};
    rec.render(&grend->cam);
    */
    // --- test

#ifndef PRESENTATION_MODE
    renderHUD();
#endif

    grend->represent();
}

void Game::renderHUD() {
    playermenu.render(&grend->cam);
    client_inventory.render(&grend->cam);
}

void Game::updateInteractiveItems() {
    for (int i = 0; i < oreProcessors.size(); ++i) {
        oreProcessors.at_index(i)->update(&droppedItems, &client_inventory);
    }
    for (int i = 0; i < dropoffAreas.size(); ++i) {
        dropoffAreas.at_index(i)->update(&droppedItems, &client_inventory);
    }
    shipbuilder.update(&droppedItems, &client_inventory);
}