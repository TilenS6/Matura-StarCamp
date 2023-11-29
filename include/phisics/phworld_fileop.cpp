#include "phisics/phisics.h"
#define FILE_VERSION 2

enum loadFromFileFlags {
    LOAD_POINTS = 1,
    LOAD_LINKS = 2,
    LOAD_MUSCELES = 4,
    LOAD_LINEOBST = 8,
    LOAD_LINKOBST = 16,
    LOAD_GRAVITY = 32,

    LOAD_MODELS_ONLY = LOAD_POINTS | LOAD_LINKS | LOAD_MUSCELES | LOAD_LINKOBST,
    LOAD_ALL = LOAD_MODELS_ONLY | LOAD_LINEOBST | LOAD_GRAVITY,
};

struct PhyWorldFileHeader {
    uint32_t phyFileCode, fileVersion; // phyFileCode = 43433
    uint32_t nPoints, nLinks, nMusceles, nLineObst, nLinkObst;
    double globalGravity;
};

/* FILE STRUCTURE:
- header: (uint32_t)fileCode = 43433, (uint32_t)fileVersion, (uint32_t)nPoints, (uint32_t)nLinks, (uint32_t)nMusceles, (uint32_t)nLineObst, (uint32_t)nLinkObst, (double)globalGravity
- data:
    nPoint-times for POINTS: (double)x, (double)y, (double)mass, (double)KoF_static, (double)KoF_kinetic, (uint32_t)collision_group_size, [collision_group_size - times] (int)collision_group; KoF_kinetic should be smaller than KoF_static
    nLinks-times for LINKS: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)original_length, (double)max_compression, (double)max_stretch; max_stretch is saved as negative value
    nMusceles-times for MUSCELES: (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)original_length, (double)max_compression, (double)max_stretch, (double)muscle_range; muscle_range: 0=no movement, 1=completelly contract (or extract to 2*length)
    nLineObst-times for OBSTACLES: (double)x1, (double)y1, (double)x2, (double)y2, (int)collision_group
    nLinkObst-times for LINK OBSTACLES: (uint32_t)link_id, (int)col_group
*/

int PhWorld::loadWorldFromFile(string fileName, uint8_t flags = LOAD_ALL, Point translation = {0, 0}, double scale = 1) { // TODO flags
    int ret = 0;

    ifstream in(fileName, ios::in | ios::binary);
    if (!in.is_open()) return -1;

    PhyWorldFileHeader fileHeader;
    if (!in.read((char *)&fileHeader.phyFileCode, sizeof(fileHeader.phyFileCode))) {
        in.close();
        return -2;
    }

    if (fileHeader.phyFileCode != 43433) {
        in.close();
        return -3;
    }
    // zdej sm sigurn da je to moja datoteka in tudi moja struktura datoteke

    in.read((char *)&fileHeader.fileVersion, sizeof(fileHeader.fileVersion));
    if (fileHeader.fileVersion != FILE_VERSION) {
        in.close();
        return -4;
    }

    if (!in.read((char *)&fileHeader.nPoints, sizeof(fileHeader.nPoints))) {
        in.close();
        return -5;
    }
    if (!in.read((char *)&fileHeader.nLinks, sizeof(fileHeader.nLinks))) {
        in.close();
        return -5;
    }
    if (!in.read((char *)&fileHeader.nMusceles, sizeof(fileHeader.nMusceles))) {
        in.close();
        return -5;
    }
    if (!in.read((char *)&fileHeader.nLineObst, sizeof(fileHeader.nLineObst))) {
        in.close();
        return -5;
    }
    if (!in.read((char *)&fileHeader.nLinkObst, sizeof(fileHeader.nLinkObst))) {
        in.close();
        return -5;
    }
    if (!in.read((char *)&fileHeader.globalGravity, sizeof(fileHeader.globalGravity))) {
        in.close();
        return -5;
    }
    FastCont<int> pointsIDs;

    //  POINTS
    for (uint32_t i = 0; i < fileHeader.nPoints; ++i) { //  (double)x, (double)y, (double)mass, (double)KoF_static, (double)KoF_kinetic, (uint32_t)collision_group_size, [collision_group_size - times] (int)collision_group
        double x, y, mass, KoF_static, KoF_kinetic;
        FastCont<int> collisionGroups(false);
        if (!in.read((char *)&x, sizeof(x))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&y, sizeof(y))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&mass, sizeof(mass))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&KoF_static, sizeof(KoF_static))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&KoF_kinetic, sizeof(KoF_kinetic))) {
            in.close();
            return -5;
        }
        uint32_t collisionGroupN;
        if (!in.read((char *)&collisionGroupN, sizeof(collisionGroupN))) {
            in.close();
            return -5;
        }
        for (uint32_t j = 0; j < collisionGroupN; ++j) {
            int tmp;
            if (!in.read((char *)&tmp, sizeof(tmp))) {
                in.close();
                return -5;
            }
            collisionGroups.push_back(tmp);
        }
        x *= scale;
        y *= scale;
        x += translation.x;
        y += translation.y;

        pointsIDs.push_back(createNewPoint(x, y, mass, collisionGroups, KoF_static, KoF_kinetic));
    }

    //  LINKS
    FastCont<int> linksIDs;
    for (uint32_t i = 0; i < fileHeader.nLinks; ++i) { //  (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)original_length, (double)max_compression, (double)max_stretch
        uint32_t p1, p2;
        double spring_koef, damp_koef, original_length, max_compression, max_stretch;
        if (!in.read((char *)&p1, sizeof(p1))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&p2, sizeof(p2))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&spring_koef, sizeof(spring_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&damp_koef, sizeof(damp_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&original_length, sizeof(original_length))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&max_compression, sizeof(max_compression))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&max_stretch, sizeof(max_stretch))) {
            in.close();
            return -5;
        }
        max_stretch *= -1;

        spring_koef /= scale;
        original_length *= scale;

        //  if data is weird... (eg. conecting point to itself, out of bounds check)
        if (p1 == p2 || p1 >= fileHeader.nPoints || p2 >= fileHeader.nPoints) {
            ret |= 1;
            continue;
        }
        linksIDs.push_back(createNewLinkBetween(*pointsIDs.at_index(p1), *pointsIDs.at_index(p2), spring_koef, damp_koef, max_compression, max_stretch, original_length));
    }

    //  MUSCELES
    for (uint32_t i = 0; i < fileHeader.nMusceles; ++i) { //  (uint32_t)p1, (uint32_t)p2, (double)spring_koef, (double)damp_koef, (double)original_length, (double)max_compression, (double)max_stretch, (double)muscle_range
        uint32_t p1, p2;
        double spring_koef, damp_koef, muscle_range, original_length, max_compression, max_stretch;
        if (!in.read((char *)&p1, sizeof(p1))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&p2, sizeof(p2))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&spring_koef, sizeof(spring_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&damp_koef, sizeof(damp_koef))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&original_length, sizeof(original_length))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&max_compression, sizeof(max_compression))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&max_stretch, sizeof(max_stretch))) {
            in.close();
            return -5;
        }
        max_stretch *= -1;
        if (!in.read((char *)&muscle_range, sizeof(muscle_range))) {
            in.close();
            return -5;
        }

        spring_koef /= scale;
        original_length *= scale;

        if (p1 == p2 || p1 >= fileHeader.nPoints || p2 >= fileHeader.nPoints || muscle_range < 0 || muscle_range > 1) {
            ret |= 2;
            continue;
        }
        createNewMuscleBetween(*pointsIDs.at_index(p1), *pointsIDs.at_index(p2), spring_koef, damp_koef, muscle_range, max_compression, max_stretch, original_length);
    }

    //  OBSTACLES
    for (uint32_t i = 0; i < fileHeader.nLineObst; ++i) { //  (double)x1, (double)y1, (double)x2, (double)y2, (int)collision_group
        double x1, y1, x2, y2;
        int col_group;
        if (!in.read((char *)&x1, sizeof(x1))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&y1, sizeof(y1))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&x2, sizeof(x2))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&y2, sizeof(y2))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&col_group, sizeof(col_group))) {
            in.close();
            return -5;
        }
        x1 *= scale;
        y1 *= scale;
        x2 *= scale;
        y2 *= scale;

        x1 += translation.x;
        y1 += translation.y;
        x2 += translation.x;
        y2 += translation.y;

        createNewLineObst(x1, y1, x2, y2, col_group);
    }

    //  LINK OBSTACLES
    for (uint32_t i = 0; i < fileHeader.nLinkObst; ++i) { //  (uint32_t)link_id, (int)col_group
        uint32_t link_id;
        int col_group;
        if (!in.read((char *)&link_id, sizeof(link_id))) {
            in.close();
            return -5;
        }
        if (!in.read((char *)&col_group, sizeof(col_group))) {
            in.close();
            return -5;
        }

        createNewLinkObst(*linksIDs.at_index(link_id), col_group);
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

void PhWorld::saveWorldToFile(string fileName) {
    ofstream out(fileName, ios::out | ios::binary);
    uint32_t fileCode = 43433;
    out.write((char *)&fileCode, sizeof(fileCode));
    uint32_t fileV = FILE_VERSION;
    out.write((char *)&fileV, sizeof(fileV));

    out.write((char *)&points.size, sizeof(points.size));
    out.write((char *)&links.size, sizeof(links.size));
    out.write((char *)&muscles.size, sizeof(muscles.size));
    out.write((char *)&lineObst.size, sizeof(lineObst.size));
    out.write((char *)&linkObst.size, sizeof(linkObst.size));

    out.write((char *)&gravity_accel, sizeof(gravity_accel));

    // POINTS
    FastCont<int> pointIDs;
    for (uint32_t i = 0; i < points.size; ++i) {
        out.write((char *)&points.at_index(i)->pos.x, sizeof(points.at_index(i)->pos.x));
        out.write((char *)&points.at_index(i)->pos.y, sizeof(points.at_index(i)->pos.y));
        out.write((char *)&points.at_index(i)->mass, sizeof(points.at_index(i)->mass));
        out.write((char *)&points.at_index(i)->KoF_static, sizeof(points.at_index(i)->KoF_static));
        out.write((char *)&points.at_index(i)->KoF_kinetic, sizeof(points.at_index(i)->KoF_kinetic));
        out.write((char *)&points.at_index(i)->collisionGroups.size, sizeof(points.at_index(i)->collisionGroups.size));
        for (uint32_t j = 0; j < points.at_index(i)->collisionGroups.size; ++j)
            out.write((char *)points.at_index(i)->collisionGroups.at_index(j), sizeof(*points.at_index(i)->collisionGroups.at_index(j)));

        pointIDs.push_back(points.get_id_at_index(i));
    }

    // LINKS
    FastCont<int> linksIDs;
    for (uint32_t i = 0; i < links.size; ++i) {
        int a = pointIDs.find_and_return_index(links.at_index(i)->idPointA);
        int b = pointIDs.find_and_return_index(links.at_index(i)->idPointB);
        out.write((char *)&a, sizeof(a));
        out.write((char *)&b, sizeof(b));
        out.write((char *)&links.at_index(i)->springKoef, sizeof(links.at_index(i)->springKoef));
        out.write((char *)&links.at_index(i)->dampKoef, sizeof(links.at_index(i)->dampKoef));
        double len = sqrt(links.at_index(i)->orgLenPow2);
        out.write((char *)&len, sizeof(len));
        out.write((char *)&links.at_index(i)->maxCompression, sizeof(links.at_index(i)->maxCompression));
        out.write((char *)&links.at_index(i)->maxStretch, sizeof(links.at_index(i)->maxStretch));

        linksIDs.push_back(links.get_id_at_index(i));
    }

    // MUSCLES
    for (uint32_t i = 0; i < muscles.size; ++i) {
        int a = pointIDs.find_and_return_index(muscles.at_index(i)->idPointA);
        int b = pointIDs.find_and_return_index(muscles.at_index(i)->idPointB);
        out.write((char *)&a, sizeof(a));
        out.write((char *)&b, sizeof(b));
        out.write((char *)&muscles.at_index(i)->springKoef, sizeof(muscles.at_index(i)->springKoef));
        out.write((char *)&muscles.at_index(i)->dampKoef, sizeof(muscles.at_index(i)->dampKoef));
        double len = sqrt(muscles.at_index(i)->orgLenPow2);
        out.write((char *)&len, sizeof(len));
        out.write((char *)&muscles.at_index(i)->maxCompression, sizeof(muscles.at_index(i)->maxCompression));
        out.write((char *)&muscles.at_index(i)->maxStretch, sizeof(muscles.at_index(i)->maxStretch));
        double min = sqrt(muscles.at_index(i)->minLenPow2);
        double max = sqrt(muscles.at_index(i)->maxLenPow2);
        double org = (min + max) / 2;
        double range = max / org - 1;
        out.write((char *)&range, sizeof(range));
    }

    // LINE OBSTACLES
    for (uint32_t i = 0; i < lineObst.size; ++i) {
        out.write((char *)&lineObst.at_index(i)->line.a.x, sizeof(lineObst.at_index(i)->line.a.x));
        out.write((char *)&lineObst.at_index(i)->line.a.y, sizeof(lineObst.at_index(i)->line.a.y));
        out.write((char *)&lineObst.at_index(i)->line.b.x, sizeof(lineObst.at_index(i)->line.b.x));
        out.write((char *)&lineObst.at_index(i)->line.b.y, sizeof(lineObst.at_index(i)->line.b.y));
        out.write((char *)&lineObst.at_index(i)->collisionGroup, sizeof(lineObst.at_index(i)->collisionGroup));
    }

    // LINK OBSTACLES
    for (uint32_t i = 0; i < linkObst.size; ++i) {
        int id = linksIDs.find_and_return_index(linkObst.at_index(i)->linkId);
        out.write((char *)&id, sizeof(id));
        out.write((char *)&linkObst.at_index(i)->collisionGroup, sizeof(linkObst.at_index(i)->collisionGroup));
    }
}