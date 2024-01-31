#include "particles/particles.h"
/*
    Rectangle r;
    Point accel;
    double accel_mult_second; // accel
    SDL_Colour colour;
*/
void Particle::init(Point spwnPnt, double size, Point initial_accel, double accel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu)
{
    r.a = spwnPnt;
    r.dimensions.x = size;
    r.dimensions.y = size;

    accel = initial_accel;
    accel_mult_second = accel_mult_per_second;
    rem_life = rem_life_seconds;
    init_life = rem_life_seconds;
    colour = {red, grn, blu, 255};
}

bool Particle::update(double dt)
{
    r.a += accel;
    accel *= 1 - ((1 - accel_mult_second) * dt);
    rem_life -= dt;
    return rem_life <= 0;
}

void Particle::render(Camera *cam)
{
    SDL_SetRenderDrawColor(cam->r, colour.r, colour.g, colour.b, 255 * (rem_life / init_life));
    r.render(cam);
}

//void ParticleS::create(Point spawner_pos, double direction, double at_speed, double life_in_sec, double interval_in_sec)
void ParticleS::create(Point spwnPnt, double size, Point initial_accel, double accel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu, double interval_in_sec)
{
    init.init(spwnPnt, size, initial_accel, accel_mult_per_second, rem_life_seconds, red, grn, blu);

    spawning = false;
    spwnTimer = 0;
    spwnRate = interval_in_sec;

    ps.reserve_n_spots(32);
}

void ParticleS::update(double dt)
{
    if (spawning)
    {
        spwnTimer += dt;
    }

    while (spwnTimer >= spwnRate)
    {
        spwnTimer -= spwnRate;
        int id = ps.push_back(Particle());
        ps.at_id(id)->init(pos, 0.1, {speed * cos(dir), speed * sin(dir)}, .5, life, 255, 0, 0);
    }

    for (int i = 0; i < ps.size; ++i)
    {
        if (ps.at_index(i)->update(dt))
        { // zelja po izbrisu
            ps.remove_index(i);
            --i;
        }
    }
}

void ParticleS::render(Camera *cam)
{
    Rectangle r;
    r.a = pos;
    r.dimensions = {size, size};
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    r.render(cam);

    for (int i = 0; i < ps.size; ++i)
    {
        ps.at_index(i)->render(cam);
    }
}