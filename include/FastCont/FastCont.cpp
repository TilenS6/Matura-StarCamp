#include "FastCont/FastCont.h"

/*
template <class T>
class FastCont {
    uint32_t size = 0, alloc_size = 0;
    T *p = nullptr;

    void push_back(T a);
    void pop_back();

    void remove(uint32_t a);
    void insert(T a, uint32_t at);

    void clear();
};
*/

template <class T>
void FastCont<T>::push_back(T a) {
    if (alloc_size == 0) {
        alloc_size = 1;
        p = (T *)malloc(sizeof(T));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (T *)realloc(p, sizeof(T) * alloc_size);
    }

    *(p + size) = a;
    ++size;
}

template <class T>
void FastCont<T>::pop_back() {
    if (size == 0) return;
    --size;
}
template <class T>
void FastCont<T>::remove(uint32_t at) {
    if (at >= size) return;
    for (uint32_t i = at + 1; i < size; ++i)
        *(p + i - 1) = *(p + i);

    --size;
}
template <class T>
void FastCont<T>::insert(T a, uint32_t at) {
    if (at > size) return;

    if (alloc_size == 0) {
        alloc_size = 1;
        p = (T *)malloc(sizeof(T));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (T *)realloc(p, sizeof(T) * alloc_size);
    }

    for (uint32_t i = size - 1; i >= at; --i)
        *(p + i + 1) = *(p + i);

    *(p + at) = a;
    ++size;
}
template <class T>
void FastCont<T>::clear() {
    free(p);
    alloc_size = 0;
    size = 0;
}
template <class T>
T FastCont<T>::operator[](uint32_t at) {
    return *(p + at);
}

template <class T>
T *FastCont<T>::at(uint32_t at) {
    return p + at;
}
