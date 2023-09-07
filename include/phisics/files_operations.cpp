#include "phisics/phisics.h"
#include <fstream>

enum loadFromFileFlags {
    LOAD_POINTS = 1,
    LOAD_LINEOBST = 2,
    LOAD_LINKS = 4,
    LOAD_MUSCELES = 8,
    LOAD_GRAVITY = 16,

    LOAD_SOLIDS = 15,
    LOAD_ALL = 31,
};

#define FILE_VERSION 1
#pragma pack(1)
struct PhyFileHeader {
    uint16_t phyFileCode, fileVersion; // phyFileCode = 43433
    uint16_t nPoints, nLineObst, nLinks, nMusceles;
    double globalGravity;
};
#pragma pack()

int loadFromFile(string fileName, PhWorld *world, uint8_t flags = LOAD_ALL) {
    ifstream in(fileName, ios::in);
    if (!in.is_open()) return -1;

    // TODO dalje

    in.close();

    return 0;
}