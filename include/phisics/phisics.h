#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include "FastCont/FastCont.h" // vector-like class FastCont<>
#include "camera/camera.h"     // Camera
#include "shapes/shapes.h"     // Point, Line, Circle

using namespace std;

#define PI 3.14159265358979323
#define PIh PI / 2
#define PI2 PI * 2

Camera *debCam = nullptr;

class PhLineObst;
class PhPoint;
class PhLink;
class PhWorld;

class PhLineObst {
public:
    Line line;
    int collisionGroup;

    PhLineObst();

    void render(Camera *);
};

class PhPoint {
    Point pos; // x: -levo +desno  ;  y: -DOL +GOR
    FastCont<bool> touchingList;
    double KoF_static;
    double KoF_kinetic;

public:
    Point force, accel;
    double mass;
    FastCont<int> collisionGroups;

    PhPoint(double, double, double, int, double, double);
    void move(double, double);

    void resolveCollisions(double, FastCont<PhLineObst> *);
    void applyChanges(double);

    void render(Camera *);

    friend class PhWorld;
    friend class PhLink;
};

class PhLink {
protected:
    PhPoint *pointA, *pointB;
    double lenPow2;

    double lastDist;

    double springKoef;
    double dampKoef;

public:
    PhLink(PhPoint *, PhPoint *, double, double);
    void update(double);
    void render(Camera *);
};

class PhMuscle : public PhLink {
    double orgLenPow2, minLenPow2, maxLenPow2;

public:
    PhMuscle(PhPoint *, PhPoint *, double, double);
    void setRange(double); // 0- no movement, 1- completelly contract, extract to 2xlen

    void expand();
    void contract();
    void relax();
    void setMuscle(double); // min(0)-max(1)
};

class PhWorld {
public:
    FastCont<PhPoint> points;
    FastCont<PhLineObst> lineObst;
    FastCont<PhLink> links;
    FastCont<PhMuscle> muscles;
    double gravity_accel = 9.81;

    int createNewPoint(double, double, double, int);
    int createNewLineObst(double, double, double, double);
    int createNewLinkBetween(int, int);
    int createNewMuscleBetween(int, int);

    void removePointByPosition(double, double, double);
    void removePointById(int);

    void applyGravity();
    void update(double);

    void render(Camera *);
};

#include "phisics/phlineobst.cpp"
#include "phisics/phpoint.cpp"
#include "phisics/phlink.cpp"
#include "phisics/phmuscle.cpp"
#include "phisics/phworld.cpp"

#include "phisics/files_operations.cpp"