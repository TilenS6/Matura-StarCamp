#pragma once
#include <iostream>
using namespace std;

template <class T>
class FastCont {
    uint32_t alloc_size = 0;
    T *p = nullptr;

public:
    uint32_t size = 0;
    
    void push_back(T);
    void pop_back();

    void remove(uint32_t);
    void insert(T, uint32_t);

    void clear();

    T operator[](uint32_t); // returns element
    T *at(uint32_t); // returns pointer
};

#include "FastCont/FastCont.cpp"