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

    virtIDs.clear();
    virtIDs.reset();

    for (int i = 0; i < 4; ++i) {
        pointIDs[i] = pointIdsForWeights[i];
        weightIds[i] = w->createNewWeightOn(pointIdsForWeights[i]);
    }
}

void FuelCont::initVirtual(FastCont<FuelCont> *p) {
    virt = true;
    fcp = p;
    empty_kg = -1;

    virtIDs.clear();
    virtIDs.reset();
}

void FuelCont::setFuel(double val) {
    if (virt) return;
    if (val >= 0 && val <= capacity)
        currentFuel = val;
}

double FuelCont::getFuel() {
    if (!virt) return currentFuel;

    double sum = 0;
    for (int i = 0; i < virtIDs.size(); ++i)
        sum += fcp->at_id(*virtIDs.at_index(i))->currentFuel;

    return sum;
}

void FuelCont::update(double dt) {
    if (virt) return;
    currentFuel += recharge * dt;
    if (currentFuel > capacity)
        currentFuel = capacity;

    double kg_per_node = (empty_kg + kg_perUnit * currentFuel) / 4;
    for (int i = 0; i < 4; ++i) {
        w->weights.at_id(weightIds[i])->changeWeight(kg_per_node);
    }
}

double FuelCont::take(double val, double *koef = nullptr) {
    if (virt) {
        double takePerCont = val / virtIDs.size();
        double k = 0;
        double virt_Ns_perUnit = 0;
        for (int i = 0; i < virtIDs.size(); ++i) {
            double tmp = 0;
            fcp->at_id(*virtIDs.at_index(i))->take(takePerCont, &tmp);
            virt_Ns_perUnit += fcp->at_id(*virtIDs.at_index(i))->Ns_perUnit;
            k += tmp;
        }
        k /= virtIDs.size();
        virt_Ns_perUnit /= virtIDs.size();
        if (koef != nullptr) *koef = k;

        return virt_Ns_perUnit * val * k;
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
    double k = currentFuel / capacity;

    // 1
    /*
    SDL_Vertex vert[6];
    int c = 0;

    unsigned char fullr = 0, fullg = 255, fullb = 0;
    unsigned char emptyr = 166, emptyg = 166, emptyb = 166;

    unsigned char r = map(k, 0, 1, emptyr, fullr);
    unsigned char g = map(k, 0, 1, emptyg, fullg);
    unsigned char b = map(k, 0, 1, emptyb, fullb);
    unsigned char a = 255;

    Point rend = w->points.at_id(pointIDs[0])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };

    rend = w->points.at_id(pointIDs[1])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };

    rend = w->points.at_id(pointIDs[2])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };

    // --

    rend = w->points.at_id(pointIDs[0])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };
    rend = w->points.at_id(pointIDs[2])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };
    rend = w->points.at_id(pointIDs[3])->getRenderPos(cam);
    vert[c++] = {
        {(float)rend.x, (float)rend.y}, // position on screen
        {r, g, b, a},                   // colour
        {(float)0, (float)0},           // texture normals
    };

    SDL_RenderGeometry(cam->r, NULL, vert, c, NULL, 0);
    */

    // 2
    /*
    double barW = 20;
    double barWpadding = 5;
    double barHpadding = 0;

    Point pos[4];
    for (int i = 0; i < 4; ++i) {
        pos[i] = w->points.at_id(pointIDs[i])->getRenderPos(cam);
    }

    Point min = pos[0], max = pos[0];
    for (int i = 1; i < 4; ++i) {
        if (min.x > pos[i].x) min.x = pos[i].x;
        if (max.x < pos[i].x) max.x = pos[i].x;
        if (min.y > pos[i].y) min.y = pos[i].y;
        if (max.y < pos[i].y) max.y = pos[i].y;
    }

    SDL_FRect border, fill;
    border.x = max.x - barW - barWpadding;
    border.y = min.y + barHpadding;
    border.w = barW;
    border.h = max.y - min.y - (2 * barHpadding);

    fill.x = border.x;
    fill.w = border.w;
    fill.h = k * border.h;
    fill.y = border.y + border.h - fill.h;

    // white bg
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    SDL_RenderFillRectF(cam->r, &border);
    // green progress
    SDL_SetRenderDrawColor(cam->r, 0, 255, 0, 255);
    SDL_RenderFillRectF(cam->r, &fill);
    // black border
    SDL_SetRenderDrawColor(cam->r, 0, 0, 0, 255);
    SDL_RenderDrawRectF(cam->r, &border);
    */
    // 3
    unsigned char fullr = 0, fullg = 255, fullb = 0;
    unsigned char emptyr = 166, emptyg = 166, emptyb = 166;

    unsigned char r = map(k, 0, 1, emptyr, fullr);
    unsigned char g = map(k, 0, 1, emptyg, fullg);
    unsigned char b = map(k, 0, 1, emptyb, fullb);
    unsigned char a = 255;

    SDL_FPoint points[5];
    for (int i = 0; i < 4; ++i) {
        Point p = w->points.at_id(pointIDs[i])->getRenderPos(cam);
        points[i].x = p.x;
        points[i].y = p.y;
    }
    points[4] = points[0];

    SDL_SetRenderDrawColor(cam->r, r, g, b, a);
    SDL_RenderDrawLinesF(cam->r, points, 5);
}