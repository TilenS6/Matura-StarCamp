#pragma once
#include <iostream>
using namespace std;

template <class T, class id_data_type>
class FastContElement {
public:
    T data;
    id_data_type id;
    FastContElement(T, id_data_type);
};

template <class T, class id_data_type = uint32_t>
class FastCont {
    uint32_t alloc_size = 0;
    id_data_type rollingID = 0;
    FastContElement<T, id_data_type>* p = nullptr;

public:
    // ~FastCont();
    uint32_t size = 0;

    id_data_type push_back(T);
    void pop_back();

    void remove_index(uint32_t);
    void remove_id(id_data_type);
    void insert(T, uint32_t);

    id_data_type get_id_at_index(uint32_t);

    void clear();

    T* at_index(uint32_t); // returns pointer to element at INDEX
    T* at_id(id_data_type); // returns pointer to element with certain ID
};

#include "FastCont/FastCont.cpp"