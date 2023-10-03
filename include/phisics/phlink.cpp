#include "phisics/phisics.h"

PhLink::PhLink(FastCont<PhPoint>* ps, int idA, int idB, double spring_koef = 50, double damp_koef = 1, double original_len = 0) {
    points = ps;
    idPointA = idA;
    idPointB = idB;

    if (original_len!=0)
        lenPow2 = original_len*original_len;
    else
        lenPow2 = distancePow2(ps->at_id(idA)->pos, ps->at_id(idB)->pos);
        
    lastDist = lenPow2;
    orgLenPow2 = lenPow2;

    springKoef = spring_koef;
    dampKoef = damp_koef;

    hasMaxComp = false;
}

void PhLink::setMaxComp(double maxCompr, double maxStr) {
    hasMaxComp = true;
    maxCompression = maxCompr;
    maxStretch = -maxStr;
}

void PhLink::makeUnbreakable() {
    hasMaxComp = false;
}

bool PhLink::update(double dt) { // returns: true on request to be deleted
    PhPoint* pointA = points->at_id(idPointA), * pointB = points->at_id(idPointB);
    if (pointA == nullptr || pointB == nullptr) {
        cout << "Err: (link) nism najdu pointov z id " << idPointA << " ali " << idPointB << "!\n";
        return false;
    }
    double dir = atan2(pointA->pos.y - pointB->pos.y, pointA->pos.x - pointB->pos.x);
    double sinDir = sin(dir), cosDir = cos(dir);

    double dx = pointA->pos.x - pointB->pos.x, dy = pointA->pos.y - pointB->pos.y;
    double distPow2 = dx * dx + dy * dy;

    double F = springKoef * .5 * (lenPow2 - distPow2);
    double Fdamp = dampKoef * ((sqrt(lastDist) - sqrt(distPow2)) / dt);

    F += Fdamp; // F+ = compression, F- = stretch
    currentForce = F;

    if (hasMaxComp) {
        double val;
        if (F >= 0) val = F/maxCompression;
        else val = F/maxStretch;

        double k = val * dt * breakingAverage_smoothingKoef;
        if (k>1) k = 1;
        else if (k<-1) k = -1;

        breakingAverage = val*k + breakingAverage*(1.-k);

        if (breakingAverage >= 1.)
            return true;
    }

    pointA->force.x += F * cosDir;
    pointA->force.y += F * sinDir;

    pointB->force.x += -F * cosDir;
    pointB->force.y += -F * sinDir;

    lastDist = distPow2;
    return false;
}
void PhLink::render(Camera* cam) {
    if (hasMaxComp) {
        double clipped = breakingAverage;
        if (clipped > 1) clipped = 1;
        if (currentForce > 0) SDL_SetRenderDrawColor(cam->r, 25 + (clipped) * 220, 25, 25, 255);
        else SDL_SetRenderDrawColor(cam->r, 25, 25, 25 + (clipped) * 220, 255);
    }
    PhPoint* pointA = points->at_id(idPointA), * pointB = points->at_id(idPointB);
    double ax = (pointA->pos.x - cam->x) * cam->scale;
    double ay = cam->h - ((pointA->pos.y - cam->y) * cam->scale);
    double bx = (pointB->pos.x - cam->x) * cam->scale;
    double by = cam->h - ((pointB->pos.y - cam->y) * cam->scale);
    if (ax < 0 || ay < 0 || bx < 0 || by < 0 || ax >= cam->w || ay >= cam->h || bx >= cam->w || by >= cam->h) return;
    SDL_RenderDrawLine(cam->r, ax, ay, bx, by);
}