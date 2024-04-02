#pragma once
#include "game.h"

#define BUILDING_BLOCK_WH 0.5

#define SHIPBUILDER_GRID_W 11
#define SHIPBUILDER_GRID_H 11

struct LinkStr {
    int idA, idB;
};
struct FuelContStr {
    int idA, idB, idC, idD;
};
struct RocketThrStr {
    int ID, facing;
};
struct Construction {
    FastCont<Point> phpoints;
    FastCont<LinkStr> links;
    FastCont<RocketThrStr> thrs;
    FastCont<FuelContStr> fuelConts;
    int seatAt = -1;
};

Construction constructions[none]{
    {{}, {}, {}, {}}, // ore_rock = 0,
    {{}, {}, {}, {}}, // ore_iron,
    {{}, {}, {}, {}}, // ore_bronze,
    {{}, {}, {}, {}}, // ore_crystal,

    {
        // building_basic,
        {
            (Point){0, 0},
            (Point){1, 0},
            (Point){1, 1},
            (Point){0, 1},
        },
        {
            (LinkStr){0, 1},
            (LinkStr){1, 2},
            (LinkStr){2, 3},
            (LinkStr){3, 0},
            (LinkStr){0, 2},
            (LinkStr){1, 3},
        },
        {},
        {},
        -1,
    },
    {
        // building_rocketthr,
        {
            (Point){0, 0},
            (Point){1 / 3., 0},
            (Point){2 / 3., 0},
            (Point){1, 0},
            (Point){1, 1},
            (Point){2 / 3., 1},
            (Point){1 / 3., 1},
            (Point){0, 1},
        },
        {
            (LinkStr){0, 1},
            (LinkStr){1, 2},
            (LinkStr){2, 3},
            (LinkStr){3, 4},
            (LinkStr){4, 5},
            (LinkStr){5, 6},
            (LinkStr){6, 7},
            (LinkStr){7, 0},

            (LinkStr){1, 3},
            (LinkStr){2, 3},
            (LinkStr){1, 4},
            (LinkStr){2, 4},

            (LinkStr){5, 0},
            (LinkStr){6, 0},
            (LinkStr){5, 3},
            (LinkStr){6, 3},
        },
        {
            (RocketThrStr){1, 6},
            (RocketThrStr){2, 5},
        },
        {},
        -1,
    },
    {
        // building_seat,
        {
            (Point){0, 0},
            (Point){1, 0},
            (Point){1, 1},
            (Point){0, 1},
            (Point){.5, .5},
        },
        {
            (LinkStr){0, 1},
            (LinkStr){1, 2},
            (LinkStr){2, 3},
            (LinkStr){3, 0},

            (LinkStr){4, 0},
            (LinkStr){4, 1},
            (LinkStr){4, 2},
            (LinkStr){4, 3},
        },
        {},
        {},
        4,
    },
    {
        // building_fuelcont,
        {
            (Point){0, 0},
            (Point){1, 0},
            (Point){1, 1},
            (Point){0, 1},
        },
        {
            (LinkStr){0, 1},
            (LinkStr){1, 2},
            (LinkStr){2, 3},
            (LinkStr){3, 0},
            (LinkStr){0, 2},
            (LinkStr){1, 3},
        },
        {},
        {(FuelContStr){0, 1, 2, 3}},
        -1,
    },
};

// #include "shipbuilder.cpp"