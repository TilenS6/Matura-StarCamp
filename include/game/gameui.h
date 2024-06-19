#include "game.h"
#include "menu/menu.h"
#pragma once

#define BTNS_MARGIN_L 50
#define BTNS_MARGIN_T 50
#define BTNS_H 50
#define BTNS_H_DIST 20
#define BTNS_W 250

struct MenuBtnsMeta {
    string prompt;
    InventoryEntry get;
    InventoryEntry need[10] = {{none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}, {none, 0}};
};

#define PLAYERMENUMETA_N 4
MenuBtnsMeta playermenuMeta[PLAYERMENUMETA_N] = {
    {"Basic Frame [1 iron]", {building_basic, 1}, {{ore_iron, 1}}},
    {"Fuel Frame [3 iron]", {building_fuelcont, 1}, {{ore_iron, 3}}},
    {"Rocket Frame [5 iron]", {building_rocketthr, 1}, {{ore_iron, 5}}},
    {"Seat Frame [2 iron]", {building_seat, 1}, {{ore_iron, 2}}},
};

class Game;
class PlayerMenu {
    bool opened;
    Button btns[PLAYERMENUMETA_N];

    Cursor c;
    double cursorSize, cursorSizeTarget;
    Text txt;

    void stateChanged();

public:
    PlayerMenu();
    void init(SDL_Renderer *, Mouse *);

    void update(Game *, double);
    void render(Camera *);

    void display(bool);
    void toggle();
    bool getState() { return opened; }
};