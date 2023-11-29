#include "phisics/phisics.h"

PhWorld::PhWorld() {
    gravity_accel = 9.81;
    points.set_memory_leak_safety(false);
    lineObst.set_memory_leak_safety(false);
    links.set_memory_leak_safety(false);
    muscles.set_memory_leak_safety(false);
    linkObst.set_memory_leak_safety(false);
    rocketThrs.set_memory_leak_safety(false);
}
void PhWorld::resetWorld() {
    gravity_accel = 9.81;

    // remove data
    points.clear();
    lineObst.clear();
    links.clear();
    muscles.clear();
    linkObst.clear();
    rocketThrs.clear();

    // resets rolling ids
    points.reset();
    lineObst.reset();
    links.reset();
    muscles.reset();
    linkObst.reset();
    rocketThrs.reset();
}

uint32_t PhWorld::createNewPoint(double x, double y, double mass, int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    tmp.touchingList.set_memory_leak_safety(false);
    tmp.collisionGroups.set_memory_leak_safety(false);
    return points.push_back(tmp);
}
uint32_t PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    tmp.touchingList.set_memory_leak_safety(false);
    tmp.collisionGroups.set_memory_leak_safety(false);
    return points.push_back(tmp);
}
uint32_t PhWorld::createNewLinkObst(int linkId, int collG = 0) {
    PhLinkObst tmp(&links);
    tmp.linkId = linkId;
    tmp.collisionGroup = collG;
    return linkObst.push_back(tmp);
}

void PhWorld::removePointById(int id) {
    for (int i = 0; i < linkObst.size; ++i) {
        if (id == links.at_id(linkObst.at_index(i)->linkId)->idPointA || id == links.at_id(linkObst.at_index(i)->linkId)->idPointB) {
            linkObst.remove_index(i);
            --i;
        }
    }
    for (int i = 0; i < links.size; ++i) {
        if (id == links.at_index(i)->idPointA || id == links.at_index(i)->idPointB) {
            links.remove_index(i);
            --i;
        }
    }
    for (int i = 0; i < muscles.size; ++i) {
        if (id == muscles.at_index(i)->idPointA || id == muscles.at_index(i)->idPointB) {
            muscles.remove_index(i);
            --i;
        }
    }

    points.remove_id(id);
}

uint32_t PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0) {
    PhLineObst tmp(x1, y1, x2, y2, coll_group);
    return lineObst.push_back(tmp);
}

uint32_t PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    PhLink tmp(&points, idA, idB, spring_koef, damp_koef, originalLength);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    links.push_back(tmp);
    return links.size - 1;
}

uint32_t PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0) {
    PhMuscle tmp(&points, idA, idB, spring_koef, damp_koef, originalLength);
    tmp.setRange(muscle_range);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    muscles.push_back(tmp);
    return muscles.size - 1;
}

uint32_t PhWorld::createNewThrOn(int attached, int facing, double thrust) {
    PhRocketThr tmp(attached, facing, thrust);
    return rocketThrs.push_back(tmp);
}

void PhWorld::applyGravity() {
    for (int i = 0; i < points.size; ++i) {
        PhPoint *pt = points.at_index(i);
        pt->force.y -= pt->mass * gravity_accel;
    }
}

void PhWorld::update(double dt) {
    for (int i = 0; i < rocketThrs.size; ++i) {
        rocketThrs.at_index(i)->update(&points);
    }

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
            if (deleteA)
                removePointById(a);
            if (deleteB)
                removePointById(b);
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
            if (deleteA)
                removePointById(a);
            if (deleteB)
                removePointById(b);
        }
    }
    for (int i = 0; i < points.size; ++i)
        points.at_index(i)->resolveCollisions(dt, &lineObst, &links, &linkObst, &points);

    // Everything is planed, apply those changes

    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->applyChanges(dt);
    }
}

void PhWorld::render(Camera *cam) {
    SDL_SetRenderDrawColor(cam->r, 200, 50, 50, 255);
    for (int i = 0; i < rocketThrs.size; ++i)
        rocketThrs.at_index(i)->render(cam, &points);

    SDL_SetRenderDrawColor(cam->r, 100, 100, 100, 255);
    for (int i = 0; i < links.size; ++i)
        links.at_index(i)->render(cam);

    SDL_SetRenderDrawColor(cam->r, 200, 100, 100, 255);
    for (int i = 0; i < muscles.size; ++i)
        muscles.at_index(i)->render(cam);

    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    for (int i = 0; i < lineObst.size; ++i)
        lineObst.at_index(i)->render(cam);
    for (int i = 0; i < points.size; ++i)
        points.at_index(i)->render(cam);
    for (int i = 0; i < linkObst.size; ++i)
        linkObst.at_index(i)->render(cam);
}

bool PhWorld::removeLinkByIds(int idA, int idB) {
    for (int i = 0; i < linkObst.size; ++i) {
        if ((idA == links.at_id(linkObst.at_index(i)->linkId)->idPointA && idB == links.at_id(linkObst.at_index(i)->linkId)->idPointB) || (idB == links.at_id(linkObst.at_index(i)->linkId)->idPointA && idA == links.at_id(linkObst.at_index(i)->linkId)->idPointB)) {
            linkObst.remove_index(i);
            --i;
        }
    }
    for (int i = 0; i < links.size; ++i) {
        PhLink *l = links.at_index(i);
        if ((l->idPointA == idA && l->idPointB == idB) || (l->idPointA == idB && l->idPointB == idA)) {
            links.remove_index(i);
            return true;
        }
    }
    return false;
}

bool PhWorld::removeMuscleByIds(int idA, int idB) {
    for (int i = 0; i < muscles.size; ++i) {
        PhLink *l = muscles.at_index(i);
        if ((l->idPointA == idA && l->idPointB == idB) || (l->idPointA == idB && l->idPointB == idA)) {
            muscles.remove_index(i);
            return true;
        }
    }
    return false;
}
bool PhWorld::removeLineObstById(int id) {
    lineObst.remove_id(id);
    return true;
}
bool PhWorld::removeLinkObstByIds(int idA, int idB) {
    for (int i = 0; i < linkObst.size; ++i) {
        if ((idA == links.at_id(linkObst.at_index(i)->linkId)->idPointA && idB == links.at_id(linkObst.at_index(i)->linkId)->idPointB) || (idB == links.at_id(linkObst.at_index(i)->linkId)->idPointA && idA == links.at_id(linkObst.at_index(i)->linkId)->idPointB)) {
            linkObst.remove_index(i);
            return true;
        }
    }
    return false;
}

void PhWorld::translateEverything(Point d) {
    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->pos += d;
    }
    for (int i = 0; i < lineObst.size; ++i) {
        lineObst.at_index(i)->line.a += d;
        lineObst.at_index(i)->line.b += d;
    }
}

//! FILE OPERATIONS ARE IN SEPERATE FILE
#include "phisics/phworld_fileop.cpp"