#include "game.h"

void ShipBuilder::init(double dx, double dy, Game *_gp) {
    g = _gp;
    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            areaGrid[y][x].setRect(dx + x * BUILDING_BLOCK_WH, dy + y * BUILDING_BLOCK_WH, BUILDING_BLOCK_WH, BUILDING_BLOCK_WH);
        }
    }
}
void ShipBuilder::build() {
#ifdef CONSOLE_LOGGING_STAGES
    cout << "buildam\n";
#endif
    BuildingBlockData gridIDs[SHIPBUILDER_GRID_H][SHIPBUILDER_GRID_W];

    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            gridIDs[y][x].ID = areaGrid[y][x].containing.entr.ID;
            gridIDs[y][x].keybind = areaGrid[y][x].thrusterKeybind;
            gridIDs[y][x].rotDeg = areaGrid[y][x].rotation;
            areaGrid[y][x].containing.entr.count--;
            if (areaGrid[y][x].containing.entr.count <= 0) {
                areaGrid[y][x].containing.entr.count = 0;
                areaGrid[y][x].containing.entr.ID = none;
            }
        }
    }

    double dx = 0, dy = 0; // TODO temp
    g->send_buildShip(*gridIDs, dx, dy, SHIPBUILDER_GRID_W, SHIPBUILDER_GRID_H);
}
void ShipBuilder::render(Camera *cam) {
    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            areaGrid[y][x].render(cam);
        }
    }
}
void ShipBuilder::update(FastCont<DroppedItem> *di, Inventory *inv = nullptr) {
    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            areaGrid[y][x].update(di, inv);
        }
    }
}
void ShipBuilder::updateHijack(Keyboard *kb, Mouse *m, Inventory *inv, Camera *cam) {
    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            areaGrid[y][x].updateHijack(kb, m, inv, cam);
        }
    }
}