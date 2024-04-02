#pragma once
#include <iostream>
using namespace std;

template <class T, class id_data_type>
struct FastContElement {
    T data;
    id_data_type id;
    FastContElement(T, id_data_type);
};

template <class T, class id_data_type = uint32_t>
class FastCont {
    uint32_t alloc_size;
    id_data_type rollingID;
    FastContElement<T, id_data_type>* p;
    bool memory_leak_safety; // free up all data on destructor, default = on

public:
    uint32_t size;

    ~FastCont();
    FastCont();
    FastCont(bool);
    FastCont(T...);
    void handleInfArgs(T, T...);
    void handleInfArgs(T);

    id_data_type push_back(T);
    void pop_back();

    void remove_index(uint32_t);
    void remove_id(id_data_type);
    void insert(T, uint32_t);

    id_data_type get_id_at_index(uint32_t);

    void clear();
    void reset(); // reset rolling IDs
    void reserve_n_spots(uint32_t);
    void set_memory_leak_safety(bool);

    T* at_index(uint32_t); // returns pointer to element at INDEX
    T* at_id(id_data_type); // returns pointer to element with certain ID

    int64_t find_and_return_index(T); // if not found, returning -1

    void force_import(id_data_type, T); // false: OK, true: ID already exists

    
};

#include "FastCont/FastCont.cpp"