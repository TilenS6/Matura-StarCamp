#include "fuelcont/fuelcont.h"

void FuelCont::init(double _capacity, double recharge_per_second) {
	capacity = _capacity;
	currentFuel = 0;
	recharge=recharge_per_second;
}
	
void FuelCont::setFuel(double val) {
	if (val>=0&&val<=capacity)
		currentFuel = val;
}

double FuelCont::getFuel() {
	return currentFuel;
}

void FuelCont::update(double dt) {
	currentFuel += recharge*dt;
	if (currentFuel>capacity)
		currentFuel = capacity;	
}

double FuelCont::take(double val) {
	if (val<=0) return 0;
	if (val<=currentFuel) {
		currentFuel-=val;
		return 1.0;
	}
	double ret=currentFuel/val;
	fuel=0;
	return ret;
}