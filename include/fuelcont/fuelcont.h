#pragma once
#include <iostream>
#include "phisics/phisics.h"

using namespace std;

class FuelCont
{
	const static double outputLimit = 0.01;
	double capacity, currentFuel;
	double recharge;
	PhWeight weight;

public:
	void init(double, double);
	void setFuel(double);
	double getFuel();

	void update(double);

	/**
	@return koeficient, kolk od tega je lahko vzel
	*/
	double take(double);
};

#include "fuelcont/fuelcont.cpp"