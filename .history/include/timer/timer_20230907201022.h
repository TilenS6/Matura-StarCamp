#pragma once
#include <iostream>
#include <chrono>

class Timer {
    std::chrono::steady_clock::time_point start, stop;

public:
    Timer();
    double interval();
};

#include "timer/timer.cpp"