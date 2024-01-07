#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <fstream>
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
class PhLinkObst;
class PhWorld;

class PhLineObst {
public:
    Line line;
    int collisionGroup;

    PhLineObst(double, double, double, double, int);

    void render(Camera *);
};

class PhPoint {
    Point pos; // x: -levo +desno  ;  y: -DOL +GOR
    FastCont<bool> touchingList;
    FastCont<bool> touchingLinksList;
    double KoF_static;
    double KoF_kinetic;
    bool virt;
    void calculateCollisions(FastCont<bool> *, int, Line, Line, Line, double, Line *);

public:
    Point force, accel;
    double mass;
    FastCont<int> collisionGroups;
    FastCont<int> virtAvgPoints;

    PhPoint(double, double, double, int, double, double);
    PhPoint(double, double, double, FastCont<int>, double, double);
    void setVirtual(bool);
    void move(double, double);

    void resolveCollisions(double, FastCont<PhLineObst> *, FastCont<PhLink> *, FastCont<PhLinkObst> *, FastCont<PhPoint> *);
    void applyChanges(double);
    void updateVirtual(PhWorld *);

    void render(Camera *);

    Point getPos() { return pos; }
    Point getRenderPos(Camera *);

    friend class PhWorld;
    friend class PhLink;
};

class PhLink {
protected:
    FastCont<PhPoint> *points;
    double lenPow2, orgLenPow2;

    double lastDist;

    double springKoef;
    double dampKoef;

    bool hasMaxComp;
    double maxCompression, maxStretch;

    double breakingAverage = 0; // breakingAverage = (val*.2)+(breakingAverage_old*.8)  --> s tem preprecim da bi se dolocen PhLink "uncil" ce je momentalno prezivel vecjo moc (1=prezivlja maxCompr. / maxStr.)
    double breakingAverage_smoothingKoef = 10;

public:
    int idPointA, idPointB;
    double currentForce;

    PhLink(FastCont<PhPoint> *, int, int, double, double, double);
    void setMaxComp(double, double);
    void makeUnbreakable();
    bool update(double);
    void render(Camera *);

    friend class PhLinkObst;
    friend class PhWorld;
};

class PhMuscle : public PhLink {
    double minLenPow2, maxLenPow2;

public:
    PhMuscle(FastCont<PhPoint> *, int, int, double, double, double);
    void setRange(double); // 0- no movement, 1- completelly contract, extract to 2xlen

    void expand();
    void contract();
    void relax();
    void setMuscle(double); // min(0)-max(1)

    friend class PhWorld;
};

class PhLinkObst {
    FastCont<PhLink> *links;

public:
    int collisionGroup;
    int linkId;

    PhLinkObst(FastCont<PhLink> *);
    void render(Camera *);
};

class PhRocketThr {
    int attachedPID;
    int facingPID;
    double dirOffset;

public:
    double currentThrust;
    double maxThrust; // [N]

    PhRocketThr(int, int, double, double);
    void relocate(int, int);
    void setState(double); // 0-1
    void update(FastCont<PhPoint> *);
    void render(Camera *, FastCont<PhPoint> *);
};

class PhWorld {
public:
    FastCont<PhPoint> points;
    FastCont<PhLineObst> lineObst;
    FastCont<PhLink> links;
    FastCont<PhMuscle> muscles;
    FastCont<PhLinkObst> linkObst;
    FastCont<PhRocketThr> rocketThrs;
    double gravity_accel;
    double accel_mult_second;

    PhWorld();

    void resetWorld();

    uint32_t createNewPoint(double, double, double, int, double, double);
    uint32_t createNewPoint(double, double, double, FastCont<int>, double, double);
    uint32_t createNewLinkBetween(int, int, double, double, double, double, double);
    uint32_t createNewMuscleBetween(int, int, double, double, double, double, double, double);
    uint32_t createNewLineObst(double, double, double, double, int);
    uint32_t createNewLinkObst(int, int);
    uint32_t createNewThrOn(int, int, double, double);

    void removePointById(int);
    bool removeLinkByIds(int, int);     // ret: TRUE on succesfull deletion
    bool removeMuscleByIds(int, int);   // ret: TRUE on succesfull deletion
    bool removeLineObstById(int);       // ret: TRUE on succesfull deletion
    bool removeLinkObstByIds(int, int); // ret: TRUE on succesfull deletion

    void translateEverything(Point);

    void applyGravity();
    void update(double);

    void render(Camera *);

    void saveWorldToFile(string);
    int loadWorldFromFile(string, uint8_t, Point, double);
    string loadWorldFromFile_getErrorMessage(int);
};

#include "phisics/phlineobst.cpp"
#include "phisics/phpoint.cpp"
#include "phisics/phlink.cpp"
#include "phisics/phmuscle.cpp"
#include "phisics/phlinkobst.cpp"
#include "phisics/phrocketthr.cpp"
#include "phisics/phworld.cpp"