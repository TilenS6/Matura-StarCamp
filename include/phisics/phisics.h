#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <fstream>
#include "FastCont/FastCont.h" // vector-like class FastCont<>
#include "camera/camera.h"     // Camera
#include "shapes/shapes.h"     // Point, Line, Circle

#include "particles/particles.h" // ParticleS

#include "game/inventory.h" // DroppedItem
#include "game/game.h"

using namespace std;

#define PI 3.14159265358979323
#define PIh PI / 2
#define PI2 PI * 2
#define SMALL_VAL (double)1e-6
#define MAX_CONTROLL_LEN 8

#define PHISICS_COLLISIONS_REQUIRED_LIFT_FORCE .01
#define PHISICS_COLLISIONS_KINETIC_FRICTION_ACCEL_THR .02
#define PHISICS_COLLISIONS_CHECK_CIRCLE_RADIUS .03

#define RENDER_TEXTURES
#define RENDER_ROCKETTHRS
// #define RENDER_LINKS
// #define RENDER_MUSCLES
#define RENDER_LINKOBST
// #define RENDER_LINEOBST
// #define RENDER_POINTS
#define RENDER_FUELCONTS
#define RENDER_PROJECTILES

Camera *debCam = nullptr;

class PhLineObst;
class PhPoint;
class PhLink;
class PhLinkObst;
class PhWeight;
class PhWorld;

class FuelCont;
class Projectile;

// external
// class Game;

bool helpers = false;

class PhLineObst {
public:
    Line line;
    int collisionGroup;

    PhLineObst(double, double, double, double, int);

    void render(Camera *);
};

class PhPoint {
    Point pos, lastPos; // x: -levo +desno  ;  y: -DOL +GOR
    FastCont<bool> touchingList;
    FastCont<bool> touchingLinksList;
    double KoF_static;
    double KoF_kinetic;
    bool virt;
    void calculateCollisions(FastCont<bool> *, int, Line, Line, Line, double, Line *);

public:
    Point force, vel, currentSpeed; // currentSpeed = prejsnji velocity
    double mass, addedMass;
    int ownership; // default: -1
    FastCont<int> collisionGroups;
    FastCont<int> virtAvgPoints;

    PhPoint(double, double, double, int, double, double);
    PhPoint(double, double, double, FastCont<int>, double, double);
    void setVirtual(bool);
    void move(double, double);

    void resolveCollisions(double, FastCont<PhLineObst> *, FastCont<PhLink> *, FastCont<PhLinkObst> *, FastCont<PhPoint> *, int *);
    void applyChanges(double);
    void updateVirtual(PhWorld *);

    void render(Camera *);

    Point getPos() { return pos; }
    Point getLastPos() { return lastPos; }
    Point getRenderPos(Camera *);

    friend class PhWorld;
    friend class PhLink;

#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
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

    // loot
    double life;
    double shielding;
    bool takeDamage(double);

public:
    int idPointA, idPointB;
    double currentForce;

    InventoryEntry loot;

    PhLink(FastCont<PhPoint> *, int, int, double, double, double);
    void setMaxComp(double, double);
    void makeUnbreakable();
    bool update(double);
    void render(Camera *);
    void setShield(double);
    double getLife() { return life; }

    friend class PhLinkObst;
    friend class PhWorld;
    friend class Projectile;

#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
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

#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

class PhLinkObst {
    FastCont<PhLink> *links;

public:
    int collisionGroup;
    int linkId;

    PhLinkObst(FastCont<PhLink> *);
    void render(Camera *);

#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

class PhRocketThr {
    int attachedPID;
    int facingPID;
    double dirOffset;

    bool psActive;

    int fuelContId;
    PhWorld *w;

public:
    ParticleS ps;
    char controlls[MAX_CONTROLL_LEN];
    int forPlayerID;

    double power; // 0-1
    // double maxThrust; // [N]
    double fuelConsumption, fuelForceMulti;

    PhRocketThr();
    void init(PhWorld *, int, int, double, double, double);
    void relocate(int, int);
    void setState(double); // 0-1
    void update(double);
    void render(Camera *);

    void setFuelSource(int);

    void initPs(double, double, double, double, double, unsigned char, unsigned char, unsigned char);

    friend class PhWorld;
#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

class PhWeight {
    PhWorld *w;
    int p;
    double addedMass;

public:
    PhWeight();
    void attachTo(PhWorld *, int);
    void changeWeight(double);

    friend class PhWorld;

#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

class FuelCont {
    PhWorld *w;
    double capacity, currentFuel;
    double recharge;

    double empty_kg, kg_perUnit;
    double Ns_perUnit;

    int weightIds[4], pointIDs[4];

    bool virt;
    FastCont<FuelCont> *fcp = nullptr;

public:
    FastCont<int> virtIDs;
    void init(double, double, PhWorld *, int[4], double, double, double);
    void initVirtual(FastCont<FuelCont> *);

    void setFuel(double);
    double getFuel();

    void update(double);
    void render(Camera *);

    /**
    @return za kolk N goriva je vzel
    */
    double take(double, double *);

    friend class PhWorld;
#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

struct PhTextureTris {
    int idA, idB, idC;
    Point normA, normB, normC;
};

class PhTexture {
    bool inited;
    FastCont<PhTextureTris> indiciesTrises; // -> SDL_Vertex(3x render_pos, 3x colour, 3x texture_normal)
    SDL_Texture *texture;
    string orgPath;

public:
    PhTexture();
    void setTexture(Camera *, string);
    void push_indicie(int, int, int, Point, Point, Point);
    void render(Camera *, PhWorld *);

    friend class PhWorld;
#ifdef GAME_EXISTS_FRIEND
    friend class Game;
#endif
};

class PhWorld {
public:
    FastCont<PhPoint> points;
    FastCont<PhLineObst> lineObst;
    FastCont<PhLink> links;
    FastCont<PhMuscle> muscles;
    FastCont<PhLinkObst> linkObst;
    FastCont<PhRocketThr> rocketThrs;
    FastCont<PhWeight> weights;
    FastCont<FuelCont> fuelConts;
    FastCont<PhTexture> textures;
    double gravity_accel;
    double vel_mult_second;

    PhWorld();

    void resetWorld();

    int createNewPoint(double, double, double, int, double, double, int);
    int createNewPoint(double, double, double, FastCont<int>, double, double, int);
    int createNewLinkBetween(int, int, double, double, double, double, double, int);
    int createNewMuscleBetween(int, int, double, double, double, double, double, double, int);
    int createNewLineObst(double, double, double, double, int, int);
    int createNewLinkObst(int, int, int);
    int createNewThrOn(int, int, double, double, double, int);
    int createNewWeightOn(int, int);
    int createNewFuelContainer(double, double, int[4], double, double, double, int);

    void removePointById(int, FastCont<int> *);
    bool removeLinkByIds(int, int);     // ret: TRUE on succesfull deletion
    bool removeLinkById(int);          // ret: TRUE on succesfull deletion
    bool removeMuscleByIds(int, int);   // ret: TRUE on succesfull deletion
    bool removeLineObstById(int);       // ret: TRUE on succesfull deletion
    bool removeLinkObstByIds(int, int); // ret: TRUE on succesfull deletion
    void removeWeightById(int);
    void removeFuelContById(int);

    void translateEverything(Point);

    void applyGravity();
    void update(double, FastCont<int> *, FastCont<int> *);

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
#include "phisics/phweight.cpp"
#include "phisics/phtexture.cpp"
#include "phisics/phworld.cpp"

#include "phisics/fuelcont.cpp"

// dodano na fonu: fuelcont, phweight