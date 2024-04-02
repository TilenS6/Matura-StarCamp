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
    int gridIDs[SHIPBUILDER_GRID_H][SHIPBUILDER_GRID_W];

    for (int y = 0; y < SHIPBUILDER_GRID_H; ++y) {
        for (int x = 0; x < SHIPBUILDER_GRID_W; ++x) {
            gridIDs[y][x] = areaGrid[y][x].getContaining().entr.ID;
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