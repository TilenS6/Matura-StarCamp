#include "particles/particles.h"
/*
    Rectangle r;
    Point accel;
    double accel_mult_second; // accel
    SDL_Colour colour;
*/
void Particle::init(Point spwnPnt, double size, Point initial_accel, double accel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu) {
    r.a = spwnPnt;
    r.dimensions.x = size;
    r.dimensions.y = size;

    accel = initial_accel;
    accel_mult_second = accel_mult_per_second;
    rem_life = rem_life_seconds;
    init_life = rem_life_seconds;
    colour = {red, grn, blu, 255};
}

bool Particle::update(double dt) {
    r.a += accel * dt;
    accel *= 1 - ((1 - accel_mult_second) * dt);
    rem_life -= dt;
    return rem_life <= 0;
}

void Particle::render(Camera *cam) {
    SDL_SetRenderDrawColor(cam->r, colour.r, colour.g, colour.b, 255 * (rem_life / init_life));
    r.render(cam);
}

// ------------------------ PS -------------------------
ParticleS::ParticleS() {
    spawning = false;
    spwnTimer = 0;
    spwnRate = 1;

    randDir = 0;
    randSpeed = 0;
    randLife = 0;

    ps.reserve_n_spots(256);
}

void ParticleS::create(Point spwnPnt, double size, double speed, double dir, double accel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu) {
    __spwnPnt = spwnPnt;
    __size = size;
    __speed = speed;
    __dir = dir;
    __accel_mult_per_second = accel_mult_per_second;
    __rem_life_seconds = rem_life_seconds;
    __red = red;
    __grn = grn;
    __blu = blu;
}

void ParticleS::setSpawnInterval(double spawn_rate) {
    spwnRate = spawn_rate;
}
void ParticleS::setRandomises(double _randDir = 0, double _randSpeed = 0, double _randLife = 0) { // TODO+  random colour
    randDir = _randDir;
    randSpeed = _randSpeed;
    randLife = _randLife;
}
void ParticleS::moveSpawner(Point new_spwnPoint, double new_dir) {
    __spwnPnt = new_spwnPoint;
    __dir = new_dir;
}

void ParticleS::update(double dt, double addMult = 1.0, Point relAccel = {0, 0}) {
    if (spawning) {
        spwnTimer += dt * addMult;
    }

    while (spwnTimer >= spwnRate) {
        spwnTimer -= spwnRate;
        Particle tmpP;
        int id = ps.push_back(tmpP);

        double tmpDir = __dir + (randDir * ((rand() % 2001) - 1000) * 0.001);
        double tmpSpd = __speed + (randSpeed * ((rand() % 2001) - 1000) * 0.001);
        ps.at_id(id)->init(__spwnPnt, __size, {(tmpSpd * cos(tmpDir)) + relAccel.x, (tmpSpd * sin(tmpDir)) + relAccel.y}, __accel_mult_per_second, __rem_life_seconds + (randLife * ((rand() % 2001) - 1000) * 0.001), __red, __grn, __blu);
        // ps.at_id(id)->accel = {spd * cos(dir), spd * sin(dir)};
    }

    for (int i = 0; i < ps.size; ++i) {
        if (ps.at_index(i)->update(dt)) { // zelja po izbrisu
            ps.remove_index(i);
            --i;
        }
    }
}

void ParticleS::render(Camera *cam) {
    /*
        Rectangle r;
        r.a = __spwnPnt;
        r.dimensions = {__size, __size};
        SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
        r.render(cam);
    */

    for (int i = 0; i < ps.size; ++i) {
        ps.at_index(i)->render(cam);
    }
}