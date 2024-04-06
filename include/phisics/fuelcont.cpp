#include "phisics/phisics.h"

void FuelCont::init(double _capacity, double recharge_per_second, PhWorld *_w, int pointIdsForWeights[4], double _empty_kg = 1, double _kg_perUnit = .1, double _Ns_perUnit = 10) {
    w = _w;
    capacity = _capacity;
    currentFuel = 0;
    recharge = recharge_per_second;

    empty_kg = _empty_kg;
    kg_perUnit = _kg_perUnit;
    Ns_perUnit = _Ns_perUnit;

    virt = false;
    fcp = nullptr;

    for (int i = 0; i < 4; ++i) {
        pointIDs[i] = pointIdsForWeights[i];
        weightIds[i] = w->createNewWeightOn(pointIdsForWeights[i]);
    }
}

void FuelCont::initVirtual(FastCont<FuelCont> *p) {
    virt = true;
    fcp = p;
    empty_kg = -1;
}

void FuelCont::setFuel(double val) {
    if (virt) return;
    if (val >= 0 && val <= capacity)
        currentFuel = val;
}

double FuelCont::getFuel() {
    if (!virt) return currentFuel;

    double sum = 0;
    for (int i = 0; i < virtIDs.size; ++i)
        sum += fcp->at_id(*virtIDs.at_index(i))->currentFuel;

    return sum;
}

void FuelCont::update(double dt) {
    if (virt) return;
    currentFuel += recharge * dt;
    if (currentFuel > capacity)
        currentFuel = capacity;
    // cout << currentFuel << endl;

    double kg_per_node = (empty_kg + kg_perUnit * currentFuel) / 4;
    for (int i = 0; i < 4; ++i) {
        w->weights.at_id(weightIds[i])->changeWeight(kg_per_node);
    }
}

double FuelCont::take(double val, double *koef = nullptr) {
    if (virt) {
        double takePerCont = val / virtIDs.size;
        double k = 0;
        for (int i = 0; i < virtIDs.size; ++i) {
            double tmp = 0;
            fcp->at_id(*virtIDs.at_index(i))->take(takePerCont, &tmp);
            k += tmp;
        }
        k /= virtIDs.size;
        if (koef != nullptr) *koef = k;

        return Ns_perUnit * val * k;
    }

    // ce ni virt:
    if (val <= 0) {
        if (koef != nullptr) *koef = 0;
        return 0;
    }
    if (val <= currentFuel) {
        currentFuel -= val;
        if (koef != nullptr) *koef = 1;
        return Ns_perUnit * val;
    }
    double ret = Ns_perUnit * currentFuel;
    if (koef != nullptr) {
        if (currentFuel == 0)
            *koef = 0;
        else
            *koef = currentFuel / val;
    }
    currentFuel = 0;
    return ret;
}

void FuelCont::render(Camera *cam) {
    if (virt) return;
    
    SDL_Vertex vert[6];
    int c = 0;

    unsigned char fullr = 0, fullg = 255, fullb = 0;
    unsigned char emptyr = 166, emptyg = 166, emptyb = 166;

    double k = currentFuel / capacity;
    unsigned char r = map(k, 0, 1, emptyr, fullr);
    unsigned char g = map(k, 0, 1, emptyg, fullg);
    unsigned char b = map(k, 0, 1, emptyb, fullb);
    unsigned char a = 255;

    Point rend = w->points.at_id(pointIDs[0])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };

    rend = w->points.at_id(pointIDs[1])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };

    rend = w->points.at_id(pointIDs[2])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };

    // --

    rend = w->points.at_id(pointIDs[0])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };
    rend = w->points.at_id(pointIDs[2])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };
    rend = w->points.at_id(pointIDs[3])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y},         // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0}, // texture normals
    };

    SDL_RenderGeometry(cam->r, NULL, vert, 6, NULL, 0);
}