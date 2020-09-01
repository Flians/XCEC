#ifndef _UTIL_H_
#define _UTIL_H_

template <typename T>
inline void safe_delete_void_ptr(void *&target) {
    if (nullptr != target) {
        T* temp = static_cast<T*>(target);
        delete temp;
        temp = nullptr;
        target = nullptr;
    }
}

#endif