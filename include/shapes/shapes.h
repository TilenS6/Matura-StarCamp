#pragma once
#include <iostream>
#include "camera/camera.h"

class Point {
public:
    double x, y;

    void render(Camera *);

    Point operator+(Point);
    Point operator-(Point);
    Point operator*(double);
    Point operator/(double);
    void operator+=(Point);
    void operator-=(Point);
    void operator*=(double);
    void operator/=(double);

    bool operator==(Point);
    bool operator!=(Point);
    void operator=(Point);
};

class Line {
public:
    Point a, b;
    void render(Camera *);

    bool operator==(Line);
    bool operator!=(Line);
    void operator=(Line);
};

class Circle {
    double r;

public:
    double rPow2;
    Point a;
    void setRadius(double);
    double getRadius();
    void render(Camera *);
};

#include "shapes/shapes.cpp"
#include "shapes/collisions.cpp"

// TODO to in shapes.cpp (koda iz collisions.cpp)