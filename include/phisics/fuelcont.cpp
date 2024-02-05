#include "phisics/phisics.h"

void FuelCont::init(double _capacity, double recharge_per_second, PhWorld *_w, int pointIdsForWeights[4], double _empty_kg = 1, double _kg_perUnit = .1, double _Ns_perUnit = 10) {
    w = _w;
    capacity = _capacity;
    currentFuel = 0;
    recharge = recharge_per_second;

    empty_kg = _empty_kg;
    kg_perUnit = _kg_perUnit;
    Ns_perUnit = _Ns_perUnit;

    for (int i = 0; i < 4; ++i) {
        pointIDs[i] = pointIdsForWeights[i];
        weightIds[i] = w->createNewWeightOn(pointIdsForWeights[i]);
    }
}

void FuelCont::setFuel(double val) {
    if (val >= 0 && val <= capacity)
        currentFuel = val;
}

double FuelCont::getFuel() {
    return currentFuel;
}

void FuelCont::update(double dt) {
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