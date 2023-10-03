#pragma once
#include <math.h>
#include "shapes/shapes.h"

#define DEBUG(x) std::cout << #x << " = " << x << std::endl;

// LINE/LINE
bool collisionLineLine(Line l1, Line l2, Point *intersection = nullptr) {
    if (l1 == l2)
        return true;
    

    double uA = ((l2.b.x - l2.a.x) * (l1.a.y - l2.a.y) - (l2.b.y - l2.a.y) * (l1.a.x - l2.a.x)) / ((l2.b.y - l2.a.y) * (l1.b.x - l1.a.x) - (l2.b.x - l2.a.x) * (l1.b.y - l1.a.y));
    double uB = ((l1.b.x - l1.a.x) * (l1.a.y - l2.a.y) - (l1.b.y - l1.a.y) * (l1.a.x - l2.a.x)) / ((l2.b.y - l2.a.y) * (l1.b.x - l1.a.x) - (l2.b.x - l2.a.x) * (l1.b.y - l1.a.y));

    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {
        if (intersection != nullptr) {
            intersection->x = l1.a.x + (uA * (l1.b.x - l1.a.x));
            intersection->y = l1.a.y + (uA * (l1.b.y - l1.a.y));
        }
        return true;
    }
    return false;
}

// POINT/CIRCLE
bool collisionPointCircle(Point p, Circle c) {
    double dx = p.x - c.a.x;
    double dy = p.y - c.a.y;
    return (dx * dx + dy * dy) <= c.rPow2;
}

// LINE/CIRLCE
bool collisionLineCircle(Line l, Circle c, Point *closest = nullptr) {
    bool vRobovih = collisionPointCircle(l.a, c) || collisionPointCircle(l.b, c);
    if (vRobovih && closest == nullptr) return true;

    double distX = l.a.x - l.b.x;
    double distY = l.a.y - l.b.y;
    double lenPow2 = (distX * distX) + (distY * distY);

    // dot product
    double dot = (((c.a.x - l.a.x) * (l.b.x - l.a.x)) + ((c.a.y - l.a.y) * (l.b.y - l.a.y))) / lenPow2;

    // najblizja tocka na crti
    double closestX = l.a.x + (dot * (l.b.x - l.a.x));
    double closestY = l.a.y + (dot * (l.b.y - l.a.y));

    if (closest != nullptr) {
        // ! to zna povzrocat tezva (se snappat na rob crte)
        if (dot < 0) {
            *closest = l.a;
        } else if (dot > 1) {
            *closest = l.b;
        } else {
            closest->x = closestX;
            closest->y = closestY;
        }
    }
    if (vRobovih) return true;
    
    if (dot < 0 || dot > 1) return false;
    

    distX = closestX - c.a.x;
    distY = closestY - c.a.y;
    double distancePow2 = (distX * distX) + (distY * distY);

    return distancePow2 <= c.rPow2;
}

bool collisionCircleCircle(Circle c1, Circle c2) {
    double dx = c1.a.x - c2.a.x;
    double dy = c1.a.y - c2.a.y;
    double distPow2 = dx * dx + dy * dy;
    double minDistPow2 = c1.getRadius() + c2.getRadius();
    minDistPow2 *= minDistPow2;

    return distPow2 <= minDistPow2;
}