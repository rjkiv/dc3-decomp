#pragma once
#include "utl/MemMgr.h"
#include <cstddef>
#include <vector>

// basically unique_ptr before unique_ptr
template <class T>
class scoped_ptr {
public:
    scoped_ptr() : mPtr(nullptr) {}
    ~scoped_ptr() { delete mPtr; }
    T *operator->() const { return mPtr; }
    operator T *() const { return mPtr; }

    void operator=(T *ptr) {
        if (mPtr != ptr) {
            delete mPtr;
            mPtr = ptr;
        }
    }

private:
    T *mPtr; // 0x0
};

template <class T>
class XboxAllocator {
public:
    typedef std::size_t size_type;

    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef const T *const_pointer;
    typedef const T &const_reference;

    template <class T2>
    struct rebind {
        typedef XboxAllocator<T2> other;
    };

    pointer allocate(const size_type count, const void *hint = nullptr) const {
        if (count != 0) {
            return reinterpret_cast<pointer>(
                MemAlloc(count * sizeof(T), __FILE__, 0x2F, "synapse", 16)
            );
        } else {
            return nullptr;
        }
    }

    void deallocate(pointer ptr, size_type count) const {
        if (ptr) {
            MemFree(ptr);
        }
    }
};

// it's aligned because XboxAllocator makes the memory 16 byte aligned
template <class T>
class aligned_vector : public std::vector<T, XboxAllocator<T> > {
public:
};
