// https://www.jeffreythompson.org/collision-detection/table_of_contents.php

#pragma once
#include <math.h>
#include "shapes/shapes.h"

#define DEBUG(x) std::cout << #x << " = " << x << std::endl;

// LINE/LINE
bool collisionLineLine(Line l1, Line l2, Point *intersection = nullptr) {
    if (l1 == l2) {
        return true;
    }

    // calculate the direction of the lines
    // calculate the distance to intersection point
    double uA = ((l2.b.x - l2.a.x) * (l1.a.y - l2.a.y) - (l2.b.y - l2.a.y) * (l1.a.x - l2.a.x)) / ((l2.b.y - l2.a.y) * (l1.b.x - l1.a.x) - (l2.b.x - l2.a.x) * (l1.b.y - l1.a.y));
    double uB = ((l1.b.x - l1.a.x) * (l1.a.y - l2.a.y) - (l1.b.y - l1.a.y) * (l1.a.x - l2.a.x)) / ((l2.b.y - l2.a.y) * (l1.b.x - l1.a.x) - (l2.b.x - l2.a.x) * (l1.b.y - l1.a.y));

    // if uA and uB are between 0-1, lines are colliding
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {

        // optionally, draw a circle where the lines meet
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
    // is either end INSIDE the circle?
    // if so, return true immediately
    bool insideEdges = collisionPointCircle(l.a, c) || collisionPointCircle(l.b, c);
    if (insideEdges && closest == nullptr) return true;

    // get length of the line
    double distX = l.a.x - l.b.x;
    double distY = l.a.y - l.b.y;
    double len = sqrt((distX * distX) + (distY * distY));

    // get dot product of the line and circle
    double dot = (((c.a.x - l.a.x) * (l.b.x - l.a.x)) + ((c.a.y - l.a.y) * (l.b.y - l.a.y))) / (len * len);

    // find the closest point on the line
    double closestX = l.a.x + (dot * (l.b.x - l.a.x));
    double closestY = l.a.y + (dot * (l.b.y - l.a.y));

    if (closest != nullptr) {
        if (dot < 0) {
            *closest = l.a;
        } else if (dot > 1) {
            *closest = l.b;
        } else {
            closest->x = closestX;
            closest->y = closestY;
        }
    }
    if (insideEdges) return true;

    // is this point actually on the line segment?
    // if so keep going, but if not, return false

    // bool onSegment = linePoint(x1, y1, x2, y2, closestX, closestY);
    bool onSegment = dot >= 0 && dot <= 1;
    if (!onSegment) return false;

    // optionally, draw a circle at the closest
    // point on the line

    // get distance to closest point
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