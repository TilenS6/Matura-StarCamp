#pragma once
#include "game/game.h"
string srvrName = "127.0.0.1";
Game::Game()
{
    phisics.gravity_accel = 0; // vesolje
    phisics.accel_mult_second = .5;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return;
    }
    wind = SDL_CreateWindow("StarCamp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0); // SDL_WINDOW_BORDERLESS namesto 0
    if (!wind)
    {
        cout << "Error creating window: " << SDL_GetError() << endl;
        SDL_Quit();
        return;
    }
    cam.assignRenderer(SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!cam.r)
    {
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
    running = true;

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
}

Game::~Game()
{
    TTF_Quit();
    SDL_DestroyRenderer(cam.r);
    SDL_DestroyWindow(wind);
    SDL_Quit();

    client.closeConnection();
}

void Game::update()
{
    double dt = t.interval();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        kb.update(event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode)
            {
            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;

            default:
                break;
            }

            break;

        case SDL_KEYUP:
            switch (event.key.keysym.scancode)
            {
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
    if (m_ev & Mouse::M_LClickMask)
    {
        cout << "L click at " << m.x << ", " << m.y << endl;
    }
    if (m_ev & Mouse::M_RClickMask)
    {
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
    for (int i = 0; i < PHISICS_SUBSTEPS; ++i)
    {
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

    if (drawRuller)
    {
        for (uint16_t y = 0, y2 = 0; y < cam.h; y += cam.scale)
        {
            if (y2)
            {
                SDL_SetRenderDrawColor(cam.r, 255, 0, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(cam.r, 0, 0, 255, 255);
            }
            y2 = !y2;
            SDL_RenderDrawLine(cam.r, 0, y, 0, y + cam.scale);
        }
    }

    SDL_RenderPresent(cam.r);
}

bool Game::networkManager() {
	int ret = client.recieveData();
	if (ret!=recieveData_NO_NEW_DATA) {
		// TODO recieve data
	}
	
	if (netRequestTimer.getTime()>=NETW_REQ_INTERVAL) {
		netRequestTimer.interval();
		// TODO send data
	}
    
    return false;
}