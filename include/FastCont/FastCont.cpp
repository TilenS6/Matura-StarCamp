#include "FastCont/FastCont.h"

template <class T, class id_data_type>
FastContElement<T, id_data_type>::FastContElement(T _data, id_data_type _id) : data(_data), id(_id) {
}

template <class T, class id_data_type>
id_data_type FastCont<T, id_data_type>::push_back(T _a) {
    // FastContElement<T, id_data_type> a(_a, rollingID++);

    if (alloc_size == 0) {
        alloc_size = 1;
        p = (FastContElement<T, id_data_type> *)malloc(sizeof(FastContElement<T, id_data_type>));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (FastContElement<T, id_data_type> *)realloc(p, sizeof(FastContElement<T, id_data_type>) * alloc_size);
    }

    (p + size)->id = rollingID++;
    // (p + size)->data = _a;
    memcpy(&(p + size)->data, &_a, sizeof(T));
    // #pragma message("talele memcpy je novi, dela kr dobr zaenkat...")
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
    for (uint32_t i = 0; i < size; ++i) {
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

    for (int64_t i = size - 1; i >= at; --i)
        *(p + i + 1) = *(p + i);

    (p + at)->data = a;
    ++size;
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::clear() {
    if (p != nullptr)
        free(p);
    p = nullptr;
    alloc_size = 0;
    size = 0;
}
template <class T, class id_data_type>
T *FastCont<T, id_data_type>::at_index(uint32_t at) {
    if (at >= size) throw invalid_argument("\"at\" out of bounds");
    return &((p + at)->data);
}

template <class T, class id_data_type>
T *FastCont<T, id_data_type>::at_id(id_data_type searchForID) {
    for (uint32_t i = 0; i < size; ++i) {
        if ((p + i)->id == searchForID) {
            return &((p + i)->data);
        }
    }
    cout << "E: FastCont::at_id\tno ID found!\n";
    return nullptr;
}

template <class T, class id_data_type>
id_data_type FastCont<T, id_data_type>::get_id_at_index(uint32_t index) {
    return (p + index)->id;
}

template <class T, class id_data_type>
void FastCont<T, id_data_type>::reserve_n_spots(uint32_t n) {
    if (alloc_size == 0) alloc_size = 1;
    while (alloc_size < n)
        alloc_size *= 2;

    p = (FastContElement<T, id_data_type> *)realloc(p, sizeof(FastContElement<T, id_data_type>) * alloc_size);
}

template <class T, class id_data_type>
void FastCont<T, id_data_type>::reset() {
    rollingID = 0;
}

template <class T, class id_data_type>
int64_t FastCont<T, id_data_type>::find_and_return_index(T a) {
    for (uint32_t i = 0; i < size; ++i) {
        if ((p + i)->data == a) return i;
    }
    return -1;
}
template <class T, class id_data_type>
void FastCont<T, id_data_type>::set_memory_leak_safety(bool a) {
    memory_leak_safety = a;
}

template <class T, class id_data_type>
FastCont<T, id_data_type>::FastCont(bool _memory_leak_safety) {
    memory_leak_safety = _memory_leak_safety;
    p = nullptr;
    rollingID = 0;
    alloc_size = 0;
    size = 0;
}
template <class T, class id_data_type>
FastCont<T, id_data_type>::FastCont() {
    memory_leak_safety = true;
    p = nullptr;
    rollingID = 0;
    alloc_size = 0;
    size = 0;
}
template <class T, class id_data_type>
FastCont<T, id_data_type>::~FastCont() {
    if (memory_leak_safety && p != nullptr)
        free(p);
}

template <class T, class id_data_type>
void FastCont<T, id_data_type>::force_import(id_data_type id, T _a) {
    if (alloc_size == 0) {
        alloc_size = 1;
        p = (FastContElement<T, id_data_type> *)malloc(sizeof(FastContElement<T, id_data_type>));
    } else if (alloc_size <= size) {
        alloc_size *= 2;
        p = (FastContElement<T, id_data_type> *)realloc(p, sizeof(FastContElement<T, id_data_type>) * alloc_size);
    }

    (p + size)->id = id;
    // (p + size)->data = _a;
    memcpy(&(p + size)->data, &_a, sizeof(T));
    ++size;

    if (id > rollingID) rollingID = id;
}