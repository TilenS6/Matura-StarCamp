#include "phisics/phisics.h"

PhWorld::PhWorld() {
    gravity_accel = 9.81;
    accel_mult_second = 1;
    points.set_memory_leak_safety(false);
    lineObst.set_memory_leak_safety(false);
    links.set_memory_leak_safety(false);
    muscles.set_memory_leak_safety(false);
    linkObst.set_memory_leak_safety(false);
    rocketThrs.set_memory_leak_safety(false);

    weights.set_memory_leak_safety(false);
    fuelConts.set_memory_leak_safety(false);
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
    weights.clear();
    fuelConts.clear();

    // resets rolling ids
    points.reset();
    lineObst.reset();
    links.reset();
    muscles.reset();
    linkObst.reset();
    rocketThrs.reset();
    weights.reset();
    fuelConts.reset();
}

int PhWorld::createNewPoint(double x, double y, double mass, int collisionGroup = 0, double static_koef = 1., double kinetic_koef = .7, int forceId = -1) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    tmp.touchingList.set_memory_leak_safety(false);
    tmp.collisionGroups.set_memory_leak_safety(false);

    if (forceId == -1)
        return points.push_back(tmp);
    points.force_import(forceId, tmp);
    return forceId;
}
int PhWorld::createNewPoint(double x, double y, double mass, FastCont<int> collisionGroup, double static_koef = 1., double kinetic_koef = .7, int forceId = -1) {
    PhPoint tmp(x, y, mass, collisionGroup, static_koef, kinetic_koef);
    tmp.touchingList.set_memory_leak_safety(false);
    tmp.collisionGroups.set_memory_leak_safety(false);

    if (forceId == -1)
        return points.push_back(tmp);
    points.force_import(forceId, tmp);
    return forceId;
}
int PhWorld::createNewLinkObst(int linkId, int collG = 0, int forceId = -1) {
    PhLinkObst tmp(&links);
    tmp.linkId = linkId;
    tmp.collisionGroup = collG;

    if (forceId == -1)
        return linkObst.push_back(tmp);
    linkObst.force_import(forceId, tmp);
    return forceId;
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
    for (int i = 0; i < weights.size; ++i) {
        if (id == weights.at_index(i)->p) {
            weights.remove_index(i);
            --i;
        }
    }

    points.remove_id(id);
}

int PhWorld::createNewLineObst(double x1, double y1, double x2, double y2, int coll_group = 0, int forceId = -1) {
    PhLineObst tmp(x1, y1, x2, y2, coll_group);

    if (forceId == -1)
        return lineObst.push_back(tmp);
    lineObst.force_import(forceId, tmp);
    return forceId;
}

int PhWorld::createNewLinkBetween(int idA, int idB, double spring_koef = 50, double damp_koef = 1, double maxCompression = 0, double maxStretch = 0, double originalLength = 0, int forceId = -1) {
    PhLink tmp(&points, idA, idB, spring_koef, damp_koef, originalLength);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    /* // ! prejsnja koda, cudna je (tale "size-1")
    links.push_back(tmp);
    return links.size - 1;
    */

    if (forceId == -1)
        return links.push_back(tmp);
    links.force_import(forceId, tmp);
    return forceId;
}

int PhWorld::createNewMuscleBetween(int idA, int idB, double spring_koef = 100, double damp_koef = 10, double muscle_range = .5, double maxCompression = 0, double maxStretch = 0, double originalLength = 0, int forceId = -1) {
    PhMuscle tmp(&points, idA, idB, spring_koef, damp_koef, originalLength);
    tmp.setRange(muscle_range);
    if (maxCompression > 0 && maxStretch > 0)
        tmp.setMaxComp(maxCompression, maxStretch);

    // ! isto ko zgori

    if (forceId == -1)
        return muscles.push_back(tmp);
    muscles.force_import(forceId, tmp);
    return forceId;
}

int PhWorld::createNewThrOn(int attached, int facing, double shift_direction, double fuelConsumption = .3, double forceMult = 1, int forceId = -1) {
    PhRocketThr tmpThr;
    tmpThr.ps.ps.set_memory_leak_safety(false);

    if (forceId == -1) {
        int id = rocketThrs.push_back(tmpThr);
        rocketThrs.at_id(id)->init(this, attached, facing, shift_direction, fuelConsumption, forceMult = 1);
        return id;
    }
    rocketThrs.force_import(forceId, tmpThr);
    rocketThrs.at_id(forceId)->init(this, attached, facing, shift_direction, fuelConsumption, forceMult = 1);
    return forceId;
}

int PhWorld::createNewWeightOn(int for_point_id, int forceId = -1) {
    PhWeight tmp;
    if (forceId == -1) {
        int id = weights.push_back(tmp);
        weights.at_id(id)->attachTo(this, for_point_id);
        return id;
    }
    weights.force_import(forceId, tmp);
    weights.at_id(forceId)->attachTo(this, for_point_id);
    return forceId;
}

int PhWorld::createNewFuelContainer(double _capacity, double recharge_per_second, int pointIdsForWeights[4], double empty_kg = 1, double kg_perFuelUnit = 1, double Ns_perFuelUnit = 50000, int forceId = -1) {
    FuelCont tmp;
    if (forceId == -1) {
        int id = fuelConts.push_back(tmp);
        fuelConts.at_id(id)->init(_capacity, recharge_per_second, this, pointIdsForWeights, empty_kg, kg_perFuelUnit, Ns_perFuelUnit);
        return id;
    }
    fuelConts.force_import(forceId, tmp);
    fuelConts.at_id(forceId)->init(_capacity, recharge_per_second, this, pointIdsForWeights, empty_kg, kg_perFuelUnit, Ns_perFuelUnit);
    return forceId;
}

void PhWorld::applyGravity() {
    for (int i = 0; i < points.size; ++i) {
        PhPoint *pt = points.at_index(i);
        pt->force.y -= pt->mass * gravity_accel;
    }
}

void PhWorld::update(double dt) {
    for (int i = 0; i < fuelConts.size; ++i) {
        fuelConts.at_index(i)->update(dt);
    }
    for (int i = 0; i < rocketThrs.size; ++i) {
        rocketThrs.at_index(i)->update(dt);
    }

    for (int i = 0; i < links.size; ++i) {
        // cout << "upd: " << i << endl;
        if (links.at_index(i)->update(dt)) { // requested self delete
            if (consoleLogging) cout << "strgam link\n";

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
            if (consoleLogging) cout << "strgam muscle\n";
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
        points.at_index(i)->accel *= 1 - ((1 - accel_mult_second) * dt);
        auto p = points.at_index(i);
    }
    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->updateVirtual(this);
    }
}

void PhWorld::render(Camera *cam) {
    SDL_SetRenderDrawColor(cam->r, 200, 50, 50, 255);
    for (int i = 0; i < rocketThrs.size; ++i) {
        rocketThrs.at_index(i)->render(cam);
    }

    SDL_SetRenderDrawColor(cam->r, 100, 100, 100, 255);
    for (int i = 0; i < links.size; ++i) {
        links.at_index(i)->render(cam);
    }

    SDL_SetRenderDrawColor(cam->r, 200, 100, 100, 255);
    for (int i = 0; i < muscles.size; ++i) {
        muscles.at_index(i)->render(cam);
    }

    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    for (int i = 0; i < lineObst.size; ++i) {
        lineObst.at_index(i)->render(cam);
    }
    for (int i = 0; i < points.size; ++i) {
        points.at_index(i)->render(cam);
    }
    for (int i = 0; i < linkObst.size; ++i) {
        linkObst.at_index(i)->render(cam);
    }
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
void PhWorld::removeWeightById(int id) {
    weights.remove_id(id);
}
void PhWorld::removeFuelContById(int id) {
    fuelConts.remove_id(id);
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