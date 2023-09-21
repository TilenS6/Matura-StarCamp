#include "FastCont/FastCont.h"

/*
template <class T>
class FastCont {
    uint32_t alloc_size = 0;
    T *p = nullptr;

public:
    ~FastCont();
    uint32_t size = 0;

    void push_back(T);
    void pop_back();

    void remove(uint32_t);
    void insert(T, uint32_t);

    void clear();

    T operator[](uint32_t); // returns element
    T *at(uint32_t); // returns pointer
};
*/
template <class T, class id_data_type>
FastContElement<T, id_data_type>::FastContElement(T _data, id_data_type _id) :
    data(_data),
    id(_id) {
}

template <class T, class id_data_type>
id_data_type FastCont<T, id_data_type>::push_back(T _a) {
    FastContElement<T, id_data_type> a(_a, rollingID++);

    if (alloc_size == 0) {
        alloc_size = 1;
        p = (FastContElement<T, id_data_type> *)malloc(sizeof(FastContElement<T, id_data_type>));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (FastContElement<T, id_data_type> *)realloc(p, sizeof(FastContElement<T, id_data_type>) * alloc_size);
    }

    *(p + size) = a;
    ++size;

    return rollingID - 1;
}

template <class T, class id_data_type>
void FastCont<T, id_data_type>::pop_back() {
    if (size == 0) return;
    --size;
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::remove_index(uint32_t at) {
    if (at >= size) return;
    for (uint32_t i = at + 1; i < size; ++i)
        *(p + i - 1) = *(p + i);

    --size;
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::remove_id(id_data_type id) {
    for (uint32_t i = 0;i < size;++i) {
        if ((p + i)->id == id) {
            remove_index(i);
        }
    }
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::insert(T a, uint32_t at) {
    if (at > size) return;

    if (alloc_size == 0) {
        alloc_size = 1;
        p = (FastContElement<T, id_data_type> *)malloc(sizeof(FastContElement<T, id_data_type>));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (FastContElement<T, id_data_type> *)realloc(p, sizeof(FastContElement<T, id_data_type>) * alloc_size);
    }

    for (uint32_t i = size - 1; i >= at; --i)
        *(p + i + 1) = *(p + i);

    *(p + at) = a;
    ++size;
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::clear() {
    free(p);
    alloc_size = 0;
    size = 0;
}
template <class T, class id_data_type>
T* FastCont<T, id_data_type>::at_index(uint32_t at) {
    return &((p + at)->data);
}

template <class T, class id_data_type>
T* FastCont<T, id_data_type>::at_id(id_data_type searchForID) {
    for (uint32_t i = 0;i < size;++i) {
        if ((p + i)->id == searchForID) {
            return &((p + i)->data);
        }
    }
    return nullptr;
}

template <class T, class id_data_type>
id_data_type FastCont<T, id_data_type>::get_id_at_index(uint32_t index) {
    return (p + index)->id;
}


// template <class T>
// FastCont<T>::~FastCont() {
//     free(p);
// }
