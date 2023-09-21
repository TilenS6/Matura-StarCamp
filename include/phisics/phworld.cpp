#include "phisics/phisics.h"

uint32_t PhWorld::createNewPoint(double x, double y, double mass, int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    return points.push_back(tmp);
}
uint32_t PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    return points.push_back(tmp);
}

void PhWorld::removePointByPosition(double x, double y, double near = .5) {
    double nearPow2 = near * near;
    double minDistPow2 = nearPow2;
    int minAt = points.size;

    for (int i = 0; i < points.size; ++i) {
        double dx = points.at_index(i)->pos.x - x, dy = points.at_index(i)->pos.y - y;
        double distPow2 = dx * dx + dy * dy;
        if (distPow2 < minDistPow2) {
            minDistPow2 = distPow2;
            minAt = i;
        }
    }

    if (minAt == points.size) return; // nothing is close to specified loc


    points.remove_index(minAt);
}

void PhWorld::removePointById(int id) {
    points.remove_id(id);

    for (int i = 0; i < links.size; ++i) {
        if (id == links.at_index(i)->idPointA || id == links.at_index(i)->idPointB)
            links.remove_index(i);
    }
    for (int i = 0; i < muscles.size; ++i) {
        if (id == muscles.at_index(i)->idPointA || id == muscles.at_index(i)->idPointB)
            links.remove_index(i);
    }
}

uint32_t PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    PhLineObst tmp(x1, y1, x2, y2, coll_group);
    return lineObst.push_back(tmp);
}

uint32_t PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0) {
    PhLink tmp(&points, idA, idB, spring_koef, damp_koef);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    links.push_back(tmp);
    return links.size - 1;
}

uint32_t PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0) {
    PhMuscle tmp(&points, idA, idB, spring_koef, damp_koef);
    tmp.setRange(muscle_range);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    muscles.push_back(tmp);
    return muscles.size - 1;
}

void PhWorld::applyGravity() {
    for (int i = 0; i < points.size; ++i) {
        PhPoint* pt = points.at_index(i);
        pt->force.y -= pt->mass * gravity_accel;
    }
}

void PhWorld::update(double dt) {
    for (int i = 0; i < links.size; ++i) {
        // cout << "upd: " << i << endl;
        if (links.at_index(i)->update(dt)) { // requested self delete
            cout << "strgam link\n";

            int a = links.at_index(i)->idPointA, b = links.at_index(i)->idPointB;
            // cout << "gledam za pointe " << a << " in " << b << endl;
            links.remove_index(i);
            --i;

            bool deleteA = true, deleteB = true;
            for (int i = 0; (i < links.size) && (deleteA || deleteB); ++i) {
                if (deleteA && (a == links.at_index(i)->idPointA || a == links.at_index(i)->idPointB))
                    deleteA = false;

                if (deleteB && (b == links.at_index(i)->idPointA || b == links.at_index(i)->idPointB))
                    deleteB = false;
            }
            for (int i = 0; i < muscles.size; ++i) {
                if (deleteA && (a == muscles.at_index(i)->idPointA || a == muscles.at_index(i)->idPointB))
                    deleteA = false;

                if (deleteB && (b == muscles.at_index(i)->idPointA || b == muscles.at_index(i)->idPointB))
                    deleteB = false;
            }
            if (deleteA) {
                // cout << "deletam se A\n";
                removePointById(a);
            }
            if (deleteB) {
                // cout << "deletam se B\n";
                removePointById(b);
            }
        }
    }

    for (int i = 0; i < muscles.size; ++i) {
        if (muscles.at_index(i)->update(dt)) { // requested self delete
            cout << "strgam muscle\n";
            int a = muscles.at_index(i)->idPointA, b = muscles.at_index(i)->idPointB;
            muscles.remove_index(i);
            --i;

            bool deleteA = true, deleteB = true;
            for (int i = 0; (i < links.size) && (deleteA || deleteB); ++i) {
                if (deleteA && (a == links.at_index(i)->idPointA || a == links.at_index(i)->idPointB))
                    deleteA = false;

                if (deleteB && (b == links.at_index(i)->idPointA || b == links.at_index(i)->idPointB))
                    deleteB = false;
            }
            for (int i = 0; i < muscles.size; ++i) {
                if (deleteA && (a == muscles.at_index(i)->idPointA || a == muscles.at_index(i)->idPointB))
                    deleteA = false;

                if (deleteB && (b == muscles.at_index(i)->idPointA || b == muscles.at_index(i)->idPointB))
                    deleteB = false;
            }
            if (deleteA) {
                // cout << "deletam se A\n";
                removePointById(a);
            }
            if (deleteB) {
                // cout << "deletam se B\n";
                removePointById(b);
            }
        }
    }
    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->resolveCollisions(dt, &lineObst);
    }

    // Everything is planed, apply those changes

    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->applyChanges(dt);
    }
}

void PhWorld::render(Camera* cam) {
    for (int i = 0; i < links.size; ++i) {
        SDL_SetRenderDrawColor(cam->r, 100, 100, 100, 255);
        links.at_index(i)->render(cam);
    }

    SDL_SetRenderDrawColor(cam->r, 200, 100, 100, 255);
    for (int i = 0; i < muscles.size; ++i)
        muscles.at_index(i)->render(cam);

    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    for (int i = 0; i < points.size; ++i)
        points.at_index(i)->render(cam);
    for (int i = 0; i < lineObst.size; ++i)
        lineObst.at_index(i)->render(cam);
}

//  -------- FILE OPERATIONS --------

enum loadFromFileFlags {
    LOAD_POINTS = 1,
    LOAD_LINKS = 2,
    LOAD_MUSCELES = 4,
    LOAD_LINEOBST = 8,
    LOAD_GRAVITY = 16,

    LOAD_MODELS_ONLY = LOAD_POINTS | LOAD_LINKS | LOAD_MUSCELES,
    LOAD_ALL = LOAD_MODELS_ONLY | LOAD_LINEOBST | LOAD_GRAVITY,
};

#define FILE_VERSION 1
struct PhyWorldFileHeader {
    uint32_t phyFileCode, fileVersion; // phyFileCode = 43433
    uint32_t nPoints, nLinks, nMusceles, nLineObst;
    double globalGravity;
};
/* FILE STRUCTURE:
- header: (uint32_t)fileCode = 43433, (uint32_t)fileVersion, (uint32_t)nPoints, (uint32_t)nLinks, (uint32_t)nMusceles, (uint32_t)nLineObst, (double)globalGravity
- data:
    nPoint-times for POINTS: (double)x, (double)y, (double)mass, (double)KoF_static, (double)KoF_kinetic, (int)collision_group; KoF_kinetic should be smaller than KoF_static
    nLinks-times for LINKS: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef
    nMusceles-times for MUSCELES: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)muscle_range; muscle_range: 0=no movement, 1=completelly contract (or extract to 2*length)
    nLineObst-times for OBSTICLES: (double)x1, (double)y1, (double)x2, (double)y2, (int)collision_group
*/

int PhWorld::loadWorldFromFile(string fileName, uint8_t flags = LOAD_ALL) {
    int ret = 0;

    ifstream in(fileName, ios::in | ios::binary);
    if (!in.is_open()) return -1;

    PhyWorldFileHeader fileHeader;
    if (!in.read((char*)&fileHeader.phyFileCode, sizeof(fileHeader.phyFileCode))) {
        in.close();
        return -2;
    }
    if (fileHeader.phyFileCode != 43433) {
        in.close();
        return -3;
    }
    // we are sure that this is our file structure

    in.read((char*)&fileHeader.fileVersion, sizeof(fileHeader.fileVersion));
    if (fileHeader.fileVersion != FILE_VERSION) {
        in.close();
        return -4;
    }

    if (!in.read((char*)&fileHeader.nPoints, sizeof(fileHeader.nPoints))) {
        in.close();
        return -5;
    }
    if (!in.read((char*)&fileHeader.nLinks, sizeof(fileHeader.nLinks))) {
        in.close();
        return -5;
    }
    if (!in.read((char*)&fileHeader.nMusceles, sizeof(fileHeader.nMusceles))) {
        in.close();
        return -5;
    }
    if (!in.read((char*)&fileHeader.nLineObst, sizeof(fileHeader.nLineObst))) {
        in.close();
        return -5;
    }
    if (!in.read((char*)&fileHeader.globalGravity, sizeof(fileHeader.globalGravity))) {
        in.close();
        return -5;
    }

    FastCont<int> pointsIDs;

    //  POINTS
    for (uint32_t i = 0; i < fileHeader.nPoints; ++i) { //  (double)x, (double)y, (double)mass, (double)KoF_static, (double)KoF_kinetic, (int)collision_group
        double x, y, mass, KoF_static, KoF_kinetic;
        FastCont<int> collisionGroups;
        if (!in.read((char*)&x, sizeof(x))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&y, sizeof(y))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&mass, sizeof(mass))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&KoF_static, sizeof(KoF_static))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&KoF_kinetic, sizeof(KoF_kinetic))) {
            in.close();
            return -5;
        }
        uint32_t collisionGroupN;
        if (!in.read((char*)&collisionGroupN, sizeof(collisionGroupN))) {
            in.close();
            return -5;
        }
        for (uint32_t j = 0; j < collisionGroupN; ++j) {
            int tmp;
            if (!in.read((char*)&tmp, sizeof(tmp))) {
                in.close();
                return -5;
            }
            collisionGroups.push_back(tmp);
        }
        pointsIDs.push_back(createNewPoint(x, y, mass, collisionGroups, KoF_static, KoF_kinetic));
    }
    //  LINKS
    for (uint32_t i = 0; i < fileHeader.nLinks; ++i) { //  (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef
        uint32_t p1, p2;
        double spring_koef, damp_koef;
        if (!in.read((char*)&p1, sizeof(p1))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&p2, sizeof(p2))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&spring_koef, sizeof(spring_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&damp_koef, sizeof(damp_koef))) {
            in.close();
            return -5;
        }

        //  if data is weird... (eg. conecting point to itself, out of bounds check)
        if (p1 == p2 || p1 >= fileHeader.nPoints || p2 >= fileHeader.nPoints) {
            ret |= 1;
            continue;
        }
        createNewLinkBetween(p1, p2, spring_koef, damp_koef);
    }
    //  MUSCELES
    for (uint32_t i = 0; i < fileHeader.nMusceles; ++i) { //  (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)muscle_range
        uint32_t p1, p2;
        double spring_koef, damp_koef, muscle_range;
        if (!in.read((char*)&p1, sizeof(p1))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&p2, sizeof(p2))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&spring_koef, sizeof(spring_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&damp_koef, sizeof(damp_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&muscle_range, sizeof(muscle_range))) {
            in.close();
            return -5;
        }

        if (p1 == p2 || p1 >= fileHeader.nPoints || p2 >= fileHeader.nPoints || muscle_range < 0 || muscle_range > 1) {
            ret |= 2;
            continue;
        }
        //!!!! not saved maxCompression, maxStretch
        // !!!
        createNewMuscleBetween(p1, p2, spring_koef, damp_koef, muscle_range);
    }

    //  OBSTICLES
    for (uint32_t i = 0; i < fileHeader.nMusceles; ++i) { //  (double)x1, (double)y1, (double)x2, (double)y2, (int)collision_group
        double x1, y1, x2, y2;
        int col_group;
        if (!in.read((char*)&x1, sizeof(x1))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&y1, sizeof(y1))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&x2, sizeof(x2))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&y2, sizeof(y2))) {
            in.close();
            return -5;
        }
        if (!in.read((char*)&col_group, sizeof(col_group))) {
            in.close();
            return -5;
        }
        createNewLineObst(x1, y1, x2, y2, col_group);
    }

    in.close();

    return ret; //  negative = error,  0 = success,  positive = warning
}

string PhWorld::loadWorldFromFile_getErrorMessage(int ret) {
    if (ret == 0) return "";
    if (ret == -1) return "Could not open the file";
    if (ret == -2) return "File is empty";
    if (ret == -3) return "Incorrect file";
    if (ret == -4) return "File version incompatable";
    if (ret < -4 || ret > 0b00000011) return "Unknown error";
    // only positives left
    string out = "Warning(s):";
    if (ret & 1) out += " corrupted data at LINKS,";
    if (ret & 2) out += " corrupted data at MUSCELES,";
    return out;
}

/* FILE STRUCTURE:
- header: (uint32_t)fileCode = 43433, (uint32_t)fileVersion, (uint32_t)nPoints, (uint32_t)nLinks, (uint32_t)nMusceles, (uint32_t)nLineObst, (double)globalGravity
- data:
    nPoint-times for POINTS: (double)x, (double)y, (double)mass, (double)KoF_static, (double)KoF_kinetic, (int)collision_group; KoF_kinetic should be smaller than KoF_static
    nLinks-times for LINKS: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef
    nMusceles-times for MUSCELES: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)muscle_range; muscle_range: 0=no movement, 1=completelly contract (or extract to 2*length)
    nLineObst-times for OBSTICLES: (double)x1, (double)y1, (double)x2, (double)y2, (int)collision_group
*/
void PhWorld::saveWorldToFile(string fileName) {
    ofstream out(fileName, ios::out | ios::binary);
    uint32_t fileCode = 43433;
    out.write((char*)&fileCode, sizeof(fileCode));

    out.write((char*)&points.size, sizeof(points.size));
    out.write((char*)&links.size, sizeof(links.size));
    out.write((char*)&muscles.size, sizeof(muscles.size));
    out.write((char*)&lineObst.size, sizeof(lineObst.size));

    // TODO veeerrryyy slow code
    out.write((char*)&gravity_accel, sizeof(gravity_accel));
    for (uint32_t i = 0; i < points.size; ++i) {
        out.write((char*)&points.at_index(i)->pos.x, sizeof(points.at_index(i)->pos.x));
        out.write((char*)&points.at_index(i)->pos.y, sizeof(points.at_index(i)->pos.y));
        out.write((char*)&points.at_index(i)->mass, sizeof(points.at_index(i)->mass));
        out.write((char*)&points.at_index(i)->KoF_static, sizeof(points.at_index(i)->KoF_static));
        out.write((char*)&points.at_index(i)->KoF_kinetic, sizeof(points.at_index(i)->KoF_kinetic));
        out.write((char*)&points.at_index(i)->collisionGroups.size, sizeof(points.at_index(i)->collisionGroups.size));
        for (uint32_t j = 0; j < points.at_index(i)->collisionGroups.size; ++j)
            out.write((char*)points.at_index(i)->collisionGroups.at_index(j), sizeof(*points.at_index(i)->collisionGroups.at_index(j)));
    }
}