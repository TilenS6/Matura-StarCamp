#include "gameui.h"

PlayerMenu::PlayerMenu() {
    opened = false;

    for (int i = 0; i < PLAYERMENUMETA_N; i++) {
        cout << playermenuMeta[i].prompt << ": " << playermenuMeta[i].get.ID << ", need:\n";
        for (int j = 0; j < 10; j++) {
            cout << "\t" << playermenuMeta[i].need[j].ID << " (" << playermenuMeta[i].need[j].count << ")\n";
        }
        cout << endl;
    }
}

void PlayerMenu::init(SDL_Renderer *r, Mouse *m) {
    for (int i = 0; i < PLAYERMENUMETA_N; i++) {
        btns[i].move(BTNS_MARGIN_L, BTNS_MARGIN_T + (BTNS_H + BTNS_H_DIST) * (i + 1), BTNS_W, BTNS_H);
        btns[i].changeText(r, playermenuMeta[i].prompt);
        btns[i].changeStyle(1);
    }

    c.init(m, r, 20);
    cursorSize = 0;
    cursorSizeTarget = 15;

    txt.changeText(r, "Crafting menu", 50);
    txt.move(30, 30);
}

void PlayerMenu::display(bool state) {
    opened = state;
    stateChanged();
}
void PlayerMenu::toggle() {
    opened = !opened;
    stateChanged();
    cout << opened << endl;
}
void PlayerMenu::stateChanged() {
    if (opened) {
        SDL_ShowCursor(SDL_DISABLE); // mam svojga
        for (int i = 0; i < PLAYERMENUMETA_N; i++) {
            btns[i].resetAnimation();
        }
        txt.resetAnimation();

    } else {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void PlayerMenu::update(Game *g, double dt) {
    cursorSizeTarget = 15;
    for (int i = 0; i < PLAYERMENUMETA_N; i++) {
        if (btns[i].hover(&g->m)) {
            cursorSizeTarget = 20;
            if (btns[i].alsoClicked()) {
                // prtisnen
                bool canBuild = true;
                for (int j = 0; j < 10; ++j) {
                    if (playermenuMeta[i].need[j].count == 0) break;

                    int searchID = playermenuMeta[i].need[j].ID;
                    int available = g->client_inventory.getAvailableByID(searchID);

                    if (available < playermenuMeta[i].need[j].count) {
                        canBuild = false;
                        break; // FOR j
                    }
                }

                if (canBuild) {
                    for (int j = 0; j < 10; ++j) {
                        if (playermenuMeta[i].need[j].count == 0) break;
                        int searchID = playermenuMeta[i].need[j].ID;
                        int need = playermenuMeta[i].need[j].count;
                        for (int k = 0; k < INVENTORY_SIZE; ++k) {
                            if (g->client_inventory.inv[k].ID == searchID && g->client_inventory.inv[k].count > 0) {
                                if (g->client_inventory.inv[k].count <= need) {
                                    need -= g->client_inventory.inv[k].count;
                                    g->client_inventory.inv[k].count = 0;
                                    g->client_inventory.inv[k].ID = none;
                                } else {
                                    g->client_inventory.inv[k].count -= need;
                                }
                                if (need <= 0) break; // FOR k
                            }
                        }
                    }

                    int rem = g->client_inventory.addItem(playermenuMeta[i].get);
                    if (rem > 0) { // adding things back
                        for (int j = 0; j < 10; ++j) {
                            g->client_inventory.addItem(playermenuMeta[i].need[j]);
                        }
                    }
                }
            }
        }
    }

    if (opened) {
        double k = pow(CURSOR_STIFFNESS, dt);
        cursorSize = cursorSize * k + cursorSizeTarget * (1. - k);
    } else {
        cursorSize = 0;
    }
    c.update(dt);
}

void PlayerMenu::render(Camera *cam) {
    if (!opened) return;

    SDL_Rect bg = {0, 0, cam->w, cam->h};
    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 200);
    SDL_RenderFillRect(cam->r, &bg);

    txt.render(cam->r);

    for (int i = 0; i < PLAYERMENUMETA_N; i++) {
        btns[i].render(cam->r);
    }
    c.render(cam->r, cursorSize);
}
