#include "shapes/shapes.h"

void Point::render(Camera *cam) {
    double tx = (x - cam->x) * cam->scale;
    double ty = (y - cam->y) * cam->scale;
    if (tx < 0 || ty < 0 || tx >= cam->w || ty >= cam->h) return;
    SDL_RenderDrawPoint(cam->r, tx, ty);
}

Point Point::operator+(Point a) {
    Point tmp;
    tmp.x = x + a.x;
    tmp.y = y + a.y;
    return tmp;
}
Point Point::operator-(Point a) {
    Point tmp;
    tmp.x = x - a.x;
    tmp.y = y - a.y;
    return tmp;
}
Point Point::operator*(double a) {
    Point tmp;
    tmp.x = x * a;
    tmp.y = y * a;
    return tmp;
}
Point Point::operator/(double a) {
    Point tmp;
    tmp.x = x / a;
    tmp.y = y / a;
    return tmp;
}
void Point::operator+=(Point a) {
    x += a.x;
    y += a.y;
}
void Point::operator-=(Point a) {
    x -= a.x;
    y -= a.y;
}
void Point::operator+=(double a) {
    x += a;
    y += a;
}
void Point::operator-=(double a) {
    x -= a;
    y -= a;
}
void Point::operator*=(double a) {
    x *= a;
    y *= a;
}
void Point::operator/=(double a) {
    x /= a;
    y /= a;
}

bool Point::operator==(Point a) {
    return x == a.x && y == a.y;
}
bool Point::operator!=(Point a) {
    return !(x == a.x && y == a.y);
}
void Point::operator=(Point a) {
    x = a.x;
    y = a.y;
}

void Line::render(Camera *cam) {
    double ax = (a.x - cam->x) * cam->scale;
    double ay = (a.y - cam->y) * cam->scale;
    double bx = (b.x - cam->x) * cam->scale;
    double by = (b.y - cam->y) * cam->scale;
    if (ax < 0 || ay < 0 || bx < 0 || by < 0 || ax >= cam->w || ay >= cam->h || bx >= cam->w || by >= cam->h) return;
    SDL_RenderDrawLine(cam->r, ax, ay, bx, by);
}

bool Line::operator==(Line L2) {
    return a == L2.a && b == L2.b;
}
bool Line::operator!=(Line L2) {
    return !(a == L2.a && b == L2.b);
}

void Line::operator=(Line L2) {
    a = L2.a;
    b = L2.b;
}

void Circle::setRadius(double _r) {
    r = _r;
    rPow2 = r * r;
}
double Circle::getRadius() { return r; }
void Circle::render(Camera *cam) {
    double ax = (a.x - cam->x) * cam->scale;
    double ay = cam->h - (a.y - cam->y) * cam->scale;
    if (ax - r > cam->w || ax + r < 0 || ay - r > cam->h || ay + r < 0) return;
    for (int y = -r; y <= r; ++y)
        for (int x = -r; x <= r; ++x)
            if (x * x + y * y <= rPow2)
                SDL_RenderDrawPoint(cam->r, ax + x, ay + y);
}

double distancePow2(Point a, Point b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}