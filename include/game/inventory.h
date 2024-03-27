#pragma once
#include "game.h"

#define INVENTORY_SIZE 9
#define INVENTORY_TEXTURE_SIZE 50
#define INVENTORY_TEXTURE_BORDER 2
#define INVENTORY_TEXTURE_BORDER_COLOUR 100, 100, 100
#define INVENTORY_TEXTURE_BORDER_COLOUR_SELECTED 255, 255, 255

#define PICKUP_RANGE 1
#define PICKUP_RANGE_POW2 PICKUP_RANGE *PICKUP_RANGE


enum InventoryIDs {
    ore_rock = 0,
    ore_iron,
    ore_bronze,
    ore_crystal,

    building_basic,
    building_rocketthr,
    building_seat,
    building_fuelcont,

    none,
};
int stackSizes[none] = {
    100, // ore_rock
    100, // ore_iron
    50,  // ore_bronze
    20,  // ore_crystal

    20, // building_basic
    20, // building_rocketthr
    5,  // building_seat
    20, // building_fuelcont
};

struct TextureMeta {
    string path, name;
};
TextureMeta inventoryMeta[none] = {
    {"./media/icons/ores/Ore_10.png", "Rock"},        // ore_rock
    {"./media/icons/ores/Ore_23.png", "Iron ore"},    // ore_iron
    {"./media/icons/ores/Ore_11.png", "Bronze ore"},  // ore_bronze
    {"./media/icons/ores/Ore_14.png", "Crystal ore"}, // ore_crystal

    {"./media/icons/constructions/Metal frame LR.png", "Basic frame"},               // building_basic
    {"./media/icons/constructions/Metal propulsion frame LR.png", "Thruster frame"}, // building_rocketthr
    {"./media/icons/constructions/Metal chair frame LR.png", "Seat frame"},          // building_seat
    {"./media/icons/constructions/Metal fuel frame LR.png", "Fuel Frame"},           // building_fuelcont
};
TextureMeta topdownMeta[none] = {
    {"./media/icons/ores/Ore_10.png", "Rock"},        // ore_rock
    {"./media/icons/ores/Ore_23.png", "Iron ore"},    // ore_iron
    {"./media/icons/ores/Ore_11.png", "Bronze ore"},  // ore_bronze
    {"./media/icons/ores/Ore_14.png", "Crystal ore"}, // ore_crystal

    {"./media/textures/building_basic.png", "Basic frame"},               // building_basic
    {"./media/textures/building_rocketthr.png", "Thruster frame"}, // building_rocketthr
    {"./media/textures/building_seat.png", "Seat frame"},          // building_seat
    {"./media/textures/building_fuelcont.png", "Fuel Frame"},           // building_fuelcont
};

struct InventoryEntry {
    int ID, count;
};
SDL_Texture* inventory_textures[none] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
SDL_Texture* topdown_textures[none] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
class Inventory {
    Timer t; // animations
    int lastSelected;

    SDL_Texture* numbers[256];
    SDL_Texture* text;
    TTF_Font* font;

public:
    InventoryEntry inv[INVENTORY_SIZE];
    int selected;

    Inventory();
    void loadTextures(Camera*);
    void render(Camera*);
};

class DroppedItem {
public:
    Point pos;
    InventoryEntry entr;
    void render(Camera*);
};

#include "inventory.cpp"