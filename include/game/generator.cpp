#include "game.h"

string controlls[8] = {
    "DE",
    "S",
    "S",
    "AQ",
    "AE",
    "W",
    "W",
    "DQ",
};

void Generator::init(Game *_g) {
    g = _g;
}
void Generator::newPlayerAt(Point transform, int forPlayerID) {
    FastCont<int> ids, thrsId;
    int centerId;

    double mult = 1. / 3.;
    double spring_hardness = 10000, spring_dampness = 30;

    // POINTS
    double p[8][2] = {
        {0.5, 1},
        {0, 2},
        {1, 3},
        {2, 3},
        {3, 2},
        {2.5, 1},
        {2.5, 0.3},
        {0.5, 0.3},
    };

    Point min = {p[0][0], p[0][1]}, max = {p[0][0], p[0][1]};
    for (int i = 0; i < 8; ++i) {
        if (min.x > p[i][0]) min.x = p[i][0];
        if (min.y > p[i][1]) min.y = p[i][1];

        if (max.x < p[i][0]) max.x = p[i][0];
        if (max.y < p[i][1]) max.y = p[i][1];
    }

    double norm[8][2] = {
        {map(1, min.x, max.x, 0, 1), map(1, min.y, max.y, 0, 1)},
        {map(0, min.x, max.x, 0, 1), map(2, min.y, max.y, 0, 1)},
        {map(1, min.x, max.x, 0, 1), map(3, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(3, min.y, max.y, 0, 1)},
        {map(3, min.x, max.x, 0, 1), map(2, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(1, min.y, max.y, 0, 1)},
        {map(2, min.x, max.x, 0, 1), map(0.3, min.y, max.y, 0, 1)},
        {map(1, min.x, max.x, 0, 1), map(0.3, min.y, max.y, 0, 1)},
    };

    // THRUSTERS
    // id1, id2, rot
    double t[8][4] = {
        {1, 4, 0},    // R
        {1, 4, -PIh}, // BW
        {4, 1, PIh},  // BW
        {4, 1, 0},    // L
        {6, 7, 0},    // L
        {6, 5, 0},    // FW
        {7, 0, 0},    // FW
        {7, 6, 0},    // R
    };

    for (int i = 0; i < 8; ++i) {
        int id = g->phisics.createNewPoint(p[i][0] * mult + transform.x, p[i][1] * mult + transform.y, 10, 0);
        g->phisics.points.at_id(id)->ownership = forPlayerID;
        ids.push_back(id);
    }

    // nastavimo center (virtualna tocka -> povprecje vseh)
    centerId = g->phisics.createNewPoint(0, 0, 0, -1, 0, 0);
    g->phisics.points.at_id(centerId)->setVirtual(true);
    for (int i = 0; i < 8; ++i) {
        g->phisics.points.at_id(centerId)->virtAvgPoints.push_back(*ids.at_index(i));
    }
    g->phisics.points.at_id(centerId)->ownership = forPlayerID;

    // nastavimo celo (virtualna tocka -> povprecje 2-3)
    int celoId = g->phisics.createNewPoint(0, 0, 0, -1, 0, 0);
    g->phisics.points.at_id(celoId)->setVirtual(true);
    g->phisics.points.at_id(celoId)->virtAvgPoints.push_back(*ids.at_index(2));
    g->phisics.points.at_id(celoId)->virtAvgPoints.push_back(*ids.at_index(3));
    g->phisics.points.at_id(celoId)->ownership = forPlayerID;

    // linke med vsemi
    for (int i = 0; i < 8; ++i) {
        g->phisics.createNewLinkBetween(*ids.at_index(i), *ids.at_index((i + 1) % 8), spring_hardness, spring_dampness);
    }
    // pa se vmes
    g->phisics.createNewLinkBetween(*ids.at_index(7), *ids.at_index(5), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(0), *ids.at_index(6), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(1), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(2), *ids.at_index(0), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(2), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(3), *ids.at_index(0), spring_hardness, spring_dampness);
    g->phisics.createNewLinkBetween(*ids.at_index(3), *ids.at_index(5), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(4), *ids.at_index(0), spring_hardness, spring_dampness);

    g->phisics.createNewLinkBetween(*ids.at_index(0), *ids.at_index(5), spring_hardness, spring_dampness);

    // thr
    int pntTmp[4] = {*ids.at_index(5), *ids.at_index(6), *ids.at_index(7), *ids.at_index(0)};
    int fuelId = g->phisics.createNewFuelContainer(1, .03, pntTmp, 1, 1.5, 10e3);
    for (int i = 0; i < 8; ++i) {
        int id = g->phisics.createNewThrOn(*ids.at_index(t[i][0]), *ids.at_index(t[i][1]), t[i][2], .01, 1);

        g->phisics.rocketThrs.at_id(id)->initPs(.05, 6, PI, .5, .3, 255, 255, 255);
        g->phisics.rocketThrs.at_id(id)->ps.setSpawnInterval(.01);
        g->phisics.rocketThrs.at_id(id)->ps.setRandomises(PI / 10, 1, .1);

        g->phisics.rocketThrs.at_id(id)->setFuelSource(fuelId);

        for (int j = 0; j < controlls[i].length(); ++j) {
            g->phisics.rocketThrs.at_id(id)->controlls[j] = controlls[i][j];
        }
        g->phisics.rocketThrs.at_id(id)->forPlayerID = forPlayerID;

        thrsId.push_back(id);
    }

    Point p_min, p_max;

    p_min = {p[0][0], p[0][1]}, p_max = p_min;
    for (int i = 1; i < 8; ++i) {

        if (p_min.x > p[i][0])
            p_min.x = p[i][0];
        if (p_min.y > p[i][1])
            p_min.y = p[i][1];

        if (p_max.x < p[i][0])
            p_max.x = p[i][0];
        if (p_max.y < p[i][1])
            p_max.y = p[i][1];
    }

    PhTexture tmpTexture;
    int tmpTid = g->phisics.textures.push_back(tmpTexture);
    PhTexture *tx = g->phisics.textures.at_id(tmpTid);

    tx->setTexture(&g->grend->cam, "./media/textures/astronaut.png");
    for (int i = 0; i < 8; ++i) {
        int i2 = (i + 1) % 8;
        Point normA = {norm[i][0], norm[i][1]};
        Point normB = {norm[i2][0], norm[i2][1]};
        Point normC = {.5, .5};
        tx->push_indicie(*ids.at_index(i), *ids.at_index(i2), centerId, normA, normB, normC);
    }
#ifdef CONSOLE_LOGGING
    cout << "pl narjen\n";
#endif
}

void Generator::planets(unsigned long seed, int count = 10) {
    PlanetGenSeed = seed;
    PlanetCount = count;
#ifdef CONSOLE_LOGGING
    cout << "generating planets, seed=" << seed << ", count=" << count << endl;
#endif

    srand(PlanetGenSeed); // vedno isti seed (clientu poslem sam talele seed)

    Planet tmp;
    int rd, gr, bl;

    double r = g->gameArea.getRadius();
    FastCont<double> zs;
    zs.reserve_n_spots(count);
    for (int i = 0; i < count; ++i) {
        double val = (50 + rand() % 201) / 100.;
        bool ins = false;

        // insertion sort
        for (int j = 0; j < zs.size(); ++j) {
            if (val > *zs.at_index(j)) {
                zs.insert(val, j);
                ins = true;
                break;
            }
        }

        if (!ins)
            zs.push_back(val);
    }

    for (int i = 0; i < count; ++i) {
        Point pos;
        do {
            pos.x = (((rand() % 201) - 100) / 100.) * r;
            pos.y = (((rand() % 201) - 100) / 100.) * r;
        } while (!collisionPointCircle(pos, g->gameArea)); // generera jih samo v playArea (krog)

        int id = g->planets.push_back(tmp);
        Planet *p = g->planets.at_id(id);
        Point3 here = {pos.x, pos.y, *zs.at_index(i)};             // zs sortiran padajoce
        bool ring = (rand() % 3) > 0;                              // 2/3 moznosti
        hsv2rgb(rand() % 360, 128 + rand() % 80, 255, rd, gr, bl); // iz graphics.h
        p->generate(&g->grend->cam, 101, 101, 10, here, rd, gr, bl, ring);
    }
}

void Generator::stars(int count = 100) {
    double r = g->gameArea.getRadius();
    r += 400;
    for (int i = 0; i < count; ++i) {
        Star s;
        int id = g->stars.push_back(s);
        Star *sp = g->stars.at_id(id);

        Point pos;
        do {
            pos.x = (((rand() % 201) - 100) / 100.) * r;
            pos.y = (((rand() % 201) - 100) / 100.) * r;
        } while (collisionPointCircle(pos, g->gameArea)); // generera jih samo v playArea (krog)
        Point3 at{pos.x, pos.y, ((rand() % 101) / 100.) * 2 + 4};
        sp->generate(&g->grend->cam, 10, 10, 3, at, 255, 255, 255, 1);
    }
}

/* GENERATOR ASTEROIDOV:
1. zberi si kje je
2. spawnej 2 tocki, dokej blizu ena druge (to bo sredina asteroida)
3. spawnej vec tock okol pa okol
4. povez jih po convex hull-u
5. pejt po vrsti po convex hull-u, povezuj te tocke na najblizjo sredinsko tocko
    - ko se talele premakne iz ene tocke na drugo jo mors povezat z obema da ne nastane kvadrat
*/

void Generator::asteroids(int count, double radius) {
    double rpow2 = radius * radius;
    Point origin = {-20, 0};
    for (int i = 0; i < count; ++i) {
        Point at;
        do {
            at.x = (rand() % (int)(radius * 2 * 100)) * 0.01 - radius;
            at.y = (rand() % (int)(radius * 2 * 100)) * 0.01 - radius;
            at += origin;
        } while (distancePow2(origin, at) > rpow2);

        int rnd = (rand() % (GENERATION_ASTEROID_POINT_C_RND * 2 + 1)) - GENERATION_ASTEROID_POINT_C_RND;
        int pointC = GENERATION_ASTEROID_POINT_C + rnd;
        int rndSize = (rand() % (GENERATION_ASTEROID_SIZE_RND * 2 + 1)) - GENERATION_ASTEROID_SIZE_RND;
        int size = GENERATION_ASTEROID_SIZE + rndSize;

        FastCont<int> PIDs;
        FastCont<Point> Ploc;
        PIDs.reserve_n_spots(pointC);

        // prva dva sta na [0, 1]
        Point noise;

        generateNewNoise(noise, GENERATION_ASTEROID_POINT_NOISE);
        double x = at.x + size / 6. + noise.x;
        double y = at.y + noise.y;
        Ploc.push_back((Point){x, y});
        PIDs.push_back(g->phisics.createNewPoint(x, y, GENERATION_ASTEROID_POINT_MASS));

        generateNewNoise(noise, GENERATION_ASTEROID_POINT_NOISE);
        x = at.x - size / 6. + noise.x;
        y = at.y + noise.y;
        Ploc.push_back((Point){x, y});
        PIDs.push_back(g->phisics.createNewPoint(x, y, GENERATION_ASTEROID_POINT_MASS));
        int lid = g->phisics.createNewLinkBetween(*PIDs.at_index(0), *PIDs.at_index(1), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
        g->phisics.createNewLinkObst(lid);

        int closerP = 0; // indeks najblizje sredinske
        for (int j = 0; j < pointC; ++j) {
            generateNewNoise(noise, GENERATION_ASTEROID_POINT_NOISE);

            double dir = ((j * PI2) / pointC) - PIh;
            x = at.x + cos(dir) * size + noise.x;
            y = at.y + sin(dir) * size + noise.y;

            Ploc.push_back((Point){x, y});
            int id = g->phisics.createNewPoint(x, y, GENERATION_ASTEROID_POINT_MASS);
            PIDs.push_back(id);
            int lid = g->phisics.createNewLinkBetween(id, *PIDs.at_index(closerP), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
            g->phisics.createNewLinkObst(lid);

            // povezava med tockami
            if (j > 0) {
                int lid = g->phisics.createNewLinkBetween(id, *PIDs.at_index(j - 1 + 2), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
                g->phisics.createNewLinkObst(lid);
                // j-1+2 => -1 prejsnji, +2 prva dva sta centra
            } else { // ta prvi
                int lid = g->phisics.createNewLinkBetween(id, *PIDs.at_index(1), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
                g->phisics.createNewLinkObst(lid);
            }

            // ta zadn
            if (j + 1 == pointC) {
                int lid = g->phisics.createNewLinkBetween(id, *PIDs.at_index(2), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
                g->phisics.createNewLinkObst(lid);
            }

            // switchanje med eno in drugo tocko
            if (j == pointC / 2) {
                closerP = 1;
                int lid = g->phisics.createNewLinkBetween(id, *PIDs.at_index(closerP), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
                g->phisics.createNewLinkObst(lid);
            }
        }

        // -------- texture
        Point min = *Ploc.at_index(0), max = *Ploc.at_index(0);
        for (int j = 1; j < Ploc.size(); ++j) {
            Point at = *Ploc.at_index(j);
            if (min.x > at.x) min.x = at.x;
            if (min.y > at.y) min.y = at.y;

            if (max.x < at.x) max.x = at.x;
            if (max.y < at.y) max.y = at.y;
        }

        closerP = 0;
        for (int j = 2; j < PIDs.size(); ++j) {
            int ida = *PIDs.at_index(j);
            int idb = *PIDs.at_index(j - 1);
            int idc = *PIDs.at_index(closerP);

            PhTexture tmp;
            int tid = g->phisics.textures.push_back(tmp);
            PhTexture *p = g->phisics.textures.at_id(tid);
            p->setTexture(&g->grend->cam, "./media/textures/asteroid_ground.png");

            // get loc
            Point normA = *Ploc.at_index(j);
            Point normB = *Ploc.at_index(j - 1);
            Point normC = *Ploc.at_index(closerP);

            // normalize
            normA.x = map(normA.x, min.x, max.x, 0, 1);
            normA.y = map(normA.y, min.y, max.y, 0, 1);
            normB.x = map(normB.x, min.x, max.x, 0, 1);
            normB.y = map(normB.y, min.y, max.y, 0, 1);
            normC.x = map(normC.x, min.x, max.x, 0, 1);
            normC.y = map(normC.y, min.y, max.y, 0, 1);

            p->push_indicie(ida, idb, idc, normA, normB, normC);

            // ta zadn
            if (j + 1 == PIDs.size()) {
                idb = *PIDs.at_index(2);
                normB = *Ploc.at_index(2);
                normB.x = map(normB.x, min.x, max.x, 0, 1);
                normB.y = map(normB.y, min.y, max.y, 0, 1);
                p->push_indicie(ida, idb, idc, normA, normB, normC);
            }

            // switchanje med eno in drugo tocko
            if (j - 1 == PIDs.size() / 2) {
                idb = *PIDs.at_index(closerP);
                normB = *Ploc.at_index(closerP);

                closerP = 1;
                idc = *PIDs.at_index(closerP);
                normC = *Ploc.at_index(closerP);

                normB.x = map(normB.x, min.x, max.x, 0, 1);
                normB.y = map(normB.y, min.y, max.y, 0, 1);
                normC.x = map(normC.x, min.x, max.x, 0, 1);
                normC.y = map(normC.y, min.y, max.y, 0, 1);

                p->push_indicie(ida, idb, idc, normA, normB, normC);
            }
        }
    }
}

/* GENERATOR ASTEROIDOV:
<vir: https://stackoverflow.com/questions/3555343/create-2d-triangles-from-2d-points>

I like svick's answer -

when implementing I would do the following

    1. calculate the lines between every pair of points
    2. Sort the list by length
    3. Remove all lines longer than your threshold
    4. Continue down the list (longest to shortest) if it crosses another line then remove it.


--------------------------


Coming a bit late to the party, but I had a similar problem (triangulation of a set of points in two dimensions with boundary constraints)...

Unfortunately, svick's algorithm combined with pondulus' suggestion has the following flaw. Consider a complete graph with five vertices as seen here.
By removing edges (longest to shortest) that intersect with other edges outside of their endpoints, one arrives at the last graph of the picture.
Clearly, the resulting set of edges cannot give rise to a triangulation. To formalise the notion of a triangulation, we are looking for a "maximal set
of non-crossing edges between points" as written in the wikipedia article on point-set triangulation.

There is an easy fix for this problem: After running svick's & pondulus' algorithm, we arrive at a set of edges which surely only consists of non-crossing
edges, but must not be maximal (as seen in the example). To make this set a triangulation, we can iterate all remaining possible edges again and add those
that do not intersect with existing ones.

Although the runtime is not great (my implementation behaves like O(n^3) for n the number of points; see here for the heuristic), I appreciate the
flexibility of including any kind of constraint on the set of allowed edges, e.g. maximum length like in Mart's problem or boundary constraints as in my case.

*/

struct twoIDs {
    int a, b;

    bool operator==(twoIDs g) {
        return a == g.a && b == g.b;
    }
    void operator=(int g) {}
    bool operator>(twoIDs g) { return false; }
};
struct threeIDs {
    int a, b, c;
};

void Generator::asteroids2(int count, double radius, Point origin = {0, 0}) {
    double rpow2 = radius * radius;
    FastCont<Point> allPos;
    for (int i = 0; i < count; ++i) {
        Point at;
    chose_again:
        do {
            at.x = (rand() % (int)(radius * 2 * 100)) * 0.01 - radius;
            at.y = (rand() % (int)(radius * 2 * 100)) * 0.01 - radius;
            at += origin;
        } while (distancePow2(origin, at) > rpow2);

        bool ok = true;
        for (int j = 0; j < allPos.size(); ++j) {
            if (distancePow2(at, *allPos.at_index(j)) <= GENERATION_ASTEROID_SPAWN_DIST * GENERATION_ASTEROID_SPAWN_DIST) {
                ok = false;
                break;
            }
        }
        if (!ok) goto chose_again;
        allPos.push_back(at);

        int rnd = (rand() % (GENERATION_ASTEROID_POINT_C_RND * 2 + 1)) - GENERATION_ASTEROID_POINT_C_RND;
        int pointC = GENERATION_ASTEROID_POINT_C + rnd;
        int rndSize = (rand() % (GENERATION_ASTEROID_SIZE_RND * 2 + 1)) - GENERATION_ASTEROID_SIZE_RND;
        int size = GENERATION_ASTEROID_SIZE + rndSize;

        FastCont<Point> P;
        FastCont<twoIDs> Conn;
        for (int j = 0; j < pointC; ++j) {
            Point pat;
            do {
                pat.x = (rand() % (int)(size * 2 * 100)) * 0.01 - size;
                pat.y = (rand() % (int)(size * 2 * 100)) * 0.01 - size;
                pat += at;
            } while (distancePow2(at, pat) > size * size);

            int id = P.push_back(pat);
#ifdef CONSOLE_LOGGING_GENERATOR
            cout << id << ":\t" << pat.x << ", " << pat.y << endl;
#endif
            for (int k = j - 1; k >= 0; --k) {
                Conn.push_back({j, k});
            }
        }

        // bbl sort
        for (int j = 0; j < Conn.size() - 1; ++j) {
            bool changed = false;
            for (int k = 0; k < Conn.size() - j - 1; k++) {
                twoIDs a = *Conn.at_index(k);
                twoIDs b = *Conn.at_index(k + 1);
                double a_dp2 = distancePow2(*P.at_index(a.a), *P.at_index(a.b));
                double b_dp2 = distancePow2(*P.at_index(b.a), *P.at_index(b.b));

                if (a_dp2 < b_dp2) {
                    twoIDs tmp = *Conn.at_index(k);
                    *Conn.at_index(k) = *Conn.at_index(k + 1);
                    *Conn.at_index(k + 1) = tmp;
                    changed = true;
                }
            }

            if (changed == false)
                break;
        }

        // eliminacija moznih povezav
        for (int j = 0; j < Conn.size(); ++j) {
            for (int k = j + 1; k < Conn.size(); ++k) {
                // IDs
                int ja = Conn.at_index(j)->a;
                int jb = Conn.at_index(j)->b;
                int ka = Conn.at_index(k)->a;
                int kb = Conn.at_index(k)->b;

                if (ja == ka || ja == kb || jb == ka || jb == kb) continue;

                Line a = {*P.at_index(ja), *P.at_index(jb)};
                Line b = {*P.at_index(ka), *P.at_index(kb)};

                if (collisionLineLine(a, b)) {
                    Conn.remove_index(j);
                    --j;
                    break;
                }
            }
        }

        // dodaj se tiste ko bi lahko nardile povezavo
        for (int j = 0; j < P.size(); ++j) {
            for (int k = j + 1; k < P.size(); ++k) {
                twoIDs possibleNew = {j, k};

                bool colliding = false;
                for (int m = 0; m < Conn.size(); ++m) {
                    int ja = possibleNew.a;
                    int jb = possibleNew.b;
                    int ka = Conn.at_index(m)->a;
                    int kb = Conn.at_index(m)->b;

                    if ((ja == ka && jb == kb) || (ja == kb && jb == ka)) {
                        colliding = true;
                        break;
                    }

                    if (ja == ka || ja == kb || jb == ka || jb == kb) continue;

                    Line a = {*P.at_index(ja), *P.at_index(jb)};
                    Line b = {*P.at_index(ka), *P.at_index(kb)};

                    if (collisionLineLine(a, b)) {
                        colliding = true;
                        break;
                    }
                }

                if (!colliding)
                    Conn.push_back(possibleNew);
            }
        }
#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "conn: " << Conn.size() << endl;
        for (int j = 0; j < Conn.size(); ++j) {
            cout << j << ": " << Conn.at_index(j)->a << ", " << Conn.at_index(j)->b << endl;
        }
#endif

        // isci trikotnike
        FastCont<threeIDs> trises;
        for (int j = 0; j < Conn.size(); ++j) {
            for (int k = j + 1; k < Conn.size(); ++k) {
                for (int m = k + 1; m < Conn.size(); ++m) {
                    twoIDs a = *Conn.at_index(j);
                    twoIDs b = *Conn.at_index(k);
                    twoIDs c = *Conn.at_index(m);

                    FastCont<int> tmp;
                    if (tmp.at_id(a.a) == nullptr) tmp.force_import(a.a, 0);
                    if (tmp.at_id(a.b) == nullptr) tmp.force_import(a.b, 0);
                    if (tmp.at_id(b.a) == nullptr) tmp.force_import(b.a, 0);
                    if (tmp.at_id(b.b) == nullptr) tmp.force_import(b.b, 0);
                    if (tmp.at_id(c.a) == nullptr) tmp.force_import(c.a, 0);
                    if (tmp.at_id(c.b) == nullptr) tmp.force_import(c.b, 0);

                    if (tmp.size() == 3) {
                        int idA = tmp.get_id_at_index(0);
                        int idB = tmp.get_id_at_index(1);
                        int idC = tmp.get_id_at_index(2);
                        bool pit = false; // point in triangle
                        for (int o = 0; o < P.size(); ++o) {
                            int pid = P.get_id_at_index(o);
                            if (pid == idA || pid == idB || pid == idC) continue;
                            if (collisionPointInTriangle(*P.at_index(o), *P.at_id(idA), *P.at_id(idB), *P.at_id(idC))) {
                                pit = true;
                                break;
                            }
                        }
                        if (!pit) {
                            threeIDs thr = {idA, idB, idC};
                            trises.push_back(thr);
                        }
                    }
                }
            }
        }
#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "trises: " << trises.size() << endl;
        for (int j = 0; j < trises.size(); ++j) {
            cout << j << ": " << trises.at_index(j)->a << ", " << trises.at_index(j)->b << ", " << trises.at_index(j)->c << endl;
        }
#endif

        FastCont<char, twoIDs> edges;
        for (int j = 0; j < trises.size(); ++j) {
            threeIDs thr = *trises.at_index(j);

            twoIDs a = {thr.a, thr.b};
            twoIDs b = {thr.b, thr.c};
            twoIDs c = {thr.c, thr.a};

            if (a.a > a.b) switchValues(a.a, a.b);
            if (b.a > b.b) switchValues(b.a, b.b);
            if (c.a > c.b) switchValues(c.a, c.b);

            if (edges.at_id(a) == nullptr) {
                edges.force_import(a, 0);
            } else {
                edges.remove_id(a);
            }
            if (edges.at_id(b) == nullptr) {
                edges.force_import(b, 0);
            } else {
                edges.remove_id(b);
            }
            if (edges.at_id(c) == nullptr) {
                edges.force_import(c, 0);
            } else {
                edges.remove_id(c);
            }
        }
#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "edges: " << edges.size() << endl;
        for (int j = 0; j < edges.size(); ++j) {
            cout << j << ": " << edges.get_id_at_index(j).a << ", " << edges.get_id_at_index(j).b << endl;
        }
#endif

        FastCont<int> edgePoints;
        for (int j = 0; j < edges.size(); ++j) {
            if (edgePoints.find_and_return_index(edges.get_id_at_index(j).a) == -1) edgePoints.push_back(edges.get_id_at_index(j).a);
            if (edgePoints.find_and_return_index(edges.get_id_at_index(j).b) == -1) edgePoints.push_back(edges.get_id_at_index(j).b);
        }
#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "edgePoints: " << edgePoints.size() << endl;
        for (int j = 0; j < edgePoints.size(); ++j) {
            cout << j << ": " << *edgePoints.at_index(j) << endl;
        }
#endif

        // racunanje mediane
        //! to rabm kasnej dol (za teksturo), ne brisat
        Point median;
        Point min = *P.at_index(0), max = *P.at_index(0);
        for (int j = 1; j < P.size(); ++j) {
            Point now = *P.at_index(j);
            if (min.x > now.x) min.x = now.x;
            if (max.x < now.x) max.x = now.x;

            if (min.y > now.y) min.y = now.y;
            if (max.y < now.y) max.y = now.y;
        }

        median = (min + max) / 2;

#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "MED: " << median.x << ", " << median.y << endl;
#endif
        struct oriID {
            double ori;
            int ID;
        };

        FastCont<oriID> ori;
        for (int j = 0; j < edgePoints.size(); ++j) {
            double dx = P.at_id(*edgePoints.at_index(j))->x - median.x;
            double dy = P.at_id(*edgePoints.at_index(j))->y - median.y;

            double val = atan2(dy, dx);

            ori.push_back({val, j});
        }

        // bbl sort
        for (int j = 0; j < ori.size() - 1; ++j) {
            bool changed = false;
            for (int k = 0; k < ori.size() - j - 1; k++) {
                double a = ori.at_index(k)->ori;
                double b = ori.at_index(k + 1)->ori;

                if (a > b) {
                    oriID tmp = *ori.at_index(k);
                    *ori.at_index(k) = *ori.at_index(k + 1);
                    *ori.at_index(k + 1) = tmp;

                    // int tmp2 = *edgePoints.at_index(k);
                    // *edgePoints.at_index(k) = *edgePoints.at_index(k + 1);
                    // *edgePoints.at_index(k + 1) = tmp2;

                    changed = true;
                }
            }

            if (changed == false)
                break;
        }

#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "ori: " << edgePoints.size() << endl;
        for (int j = 0; j < ori.size(); ++j) {
            cout << j << ": " << ori.at_index(j)->ID << " => " << ori.at_index(j)->ori << endl;
        }
#endif
        // povezovanje "ori" v linke
        FastCont<twoIDs> borderConn;
        for (int j = 0; j < ori.size(); ++j) {
            int jj = j - 1;
            if (jj < 0) jj = ori.size() - 1;
            twoIDs tmp = {*edgePoints.at_id(ori.at_index(j)->ID), *edgePoints.at_id(ori.at_index(jj)->ID)};
            borderConn.push_back(tmp);

            // brisanje v Conn, da ne bo podvojitev
            for (int k = 0; k < Conn.size(); ++k) {
                twoIDs tmp2 = *Conn.at_index(k);
                if (tmp == tmp2 || (tmp.a == tmp2.b && tmp.b == tmp2.a)) {
                    Conn.remove_index(k);
                    --k;
                }
            }
        }

#ifdef CONSOLE_LOGGING_GENERATOR
        cout << "borderConn\n";
        for (int j = 0; j < borderConn.size(); ++j) {
            cout << j << ":: " << borderConn.at_index(j)->a << "-" << borderConn.at_index(j)->b << endl;
        }
#endif

        FastCont<int> PIDs;
        for (int j = 0; j < P.size(); ++j) {
            int pid = g->phisics.createNewPoint(P.at_index(j)->x, P.at_index(j)->y, GENERATION_ASTEROID_POINT_MASS, 1);
            PIDs.push_back(pid);
        }

        for (int j = 0; j < Conn.size(); ++j) {
            int lid = g->phisics.createNewLinkBetween(*PIDs.at_index(Conn.at_index(j)->a), *PIDs.at_index(Conn.at_index(j)->b), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
            InventoryEntry loot;
            loot.count = 1;
            loot.ID = ore_rock;
            g->phisics.links.at_id(lid)->loot = loot;
            // g->phisics.createNewLinkObst(lid);
        }

        for (int j = 0; j < borderConn.size(); ++j) {
            int lid = g->phisics.createNewLinkBetween(*PIDs.at_index(borderConn.at_index(j)->a), *PIDs.at_index(borderConn.at_index(j)->b), GENERATION_ASTEROID_SPRING_KOEF, GENERATION_ASTEROID_DAMP_KOEF);
            g->phisics.createNewLinkObst(lid);
            InventoryEntry loot;
            loot.count = 1;
            loot.ID = ore_rock;
            g->phisics.links.at_id(lid)->loot = loot;
        }

        // -------- texture

        //* min-max: mediana ze od prej

        PhTexture tmp;
        int tid = g->phisics.textures.push_back(tmp);
        PhTexture *p = g->phisics.textures.at_id(tid);
        p->setTexture(&g->grend->cam, "./media/textures/asteroid_ground.png");

        for (int j = 0; j < trises.size(); ++j) {
            threeIDs ids = *trises.at_index(j);

            // get loc
            Point normA = *P.at_index(ids.a);
            Point normB = *P.at_index(ids.b);
            Point normC = *P.at_index(ids.c);

            // normalize
            normA.x = map(normA.x, min.x, max.x, 0, 1);
            normA.y = map(normA.y, min.y, max.y, 1, 0);
            normB.x = map(normB.x, min.x, max.x, 0, 1);
            normB.y = map(normB.y, min.y, max.y, 1, 0);
            normC.x = map(normC.x, min.x, max.x, 0, 1);
            normC.y = map(normC.y, min.y, max.y, 1, 0);

            p->push_indicie(*PIDs.at_index(ids.a), *PIDs.at_index(ids.b), *PIDs.at_index(ids.c), normA, normB, normC);
        }
    }
}