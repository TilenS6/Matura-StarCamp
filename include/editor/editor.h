#pragma once
#include <iostream>
#include <vector>
#include "phisics/phisics.h"
#include "camera/camera.h"

#define MAXELDATA 10
class ElementOptions {
public:
    double data[MAXELDATA];

    void inputData(string, Camera *);
};

#include "editor/editor.cpp"