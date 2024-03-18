#pragma once

#define INVENTORY_SIZE 9
#define INVENTORY_TEXTURE_SIZE 50
#define INVENTORY_TEXTURE_BORDER 2
#define INVENTORY_TEXTURE_BORDER_COLOUR 100, 100, 100
#define INVENTORY_TEXTURE_BORDER_COLOUR_SELECTED 255, 255, 255

#define PICKUP_RANGE 1
#define PICKUP_RANGE_POW2 PICKUP_RANGE*PICKUP_RANGE


enum InventoryIDs {
    ore_rock = 0,
    ore_iron,
    ore_gold,
    ore_diamond,

    building_basic,
    building_rocketthr,
    building_seat,
    building_fuelcont,

    none,
};
int stackSizes[none] = {
    100, // ore_rock
    100, // ore_iron
    50, // ore_gold
    20, // ore_diamond

    20, // building_basic
    20, // building_rocketthr
    5, // building_seat
    20, // building_fuelcont
};


struct InventoryEntry {
    int ID, count;
};
class Inventory {
public:
    InventoryEntry inv[INVENTORY_SIZE];
    int selected;

    Inventory();
    void render(Camera *);
};

struct DroppedItem {
    Point pos;
    InventoryEntry entr;
};

#include "inventory.cpp"