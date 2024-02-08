
#pragma ignore
#include "game/game.h"
/*
    Rectangle r;
    Point vel;
    double vel_mult_second; // vel
    SDL_Colour colour;
*/
void Particle::init(Point rect, double size, Point initial_vel, double vel_mult_per_second, double rem_life_seconds, unsigned char red, unsigned char grn, unsigned char blu) {
    r.a = rect;
    r.dimensions.x = size;
    r.dimensions.y = size;

    vel = initial_vel;
    vel_mult_second = vel_mult_per_second;
    rem_life = rem_life_seconds;
    init_life = rem_life_seconds;
    colour = {red, grn, blu, 255};
}

bool Particle::update(double dt) {
    r.a += vel;
    vel *= 1 - ((1 - vel_mult_second) * dt);
    rem_life -= dt;
    return rem_life <= 0;
}

void Particle::render(Camera *cam) {
    SDL_SetRenderDrawColor(cam->r, colour.r, colour.g, colour.b, 255 * (rem_life / init_life));
    r.render(cam);
}

ParticleS::ParticleS(Point spawner_pos, double direction, double at_speed, double life_in_sec) {
    pos = spawner_pos;


    dir = direction;
    speed = at_speed;
    life = life_in_sec;
    
    randDir = 0;
    randSpeed = 0;
    randLife = 0;

    ps.reserve_n_spots(32);
}