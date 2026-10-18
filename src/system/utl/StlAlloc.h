#pragma once
#include <system/utl/MemMgr.h>
#include <typeinfo>

#ifdef STLPORT
// StlNodeAlloc exists in the STLport namespace
namespace STLPORT {
#endif

    template <class T>
    class StlNodeAlloc {
    public:
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef const T *const_pointer;
        typedef const T &const_reference;

        template <class T2>
        struct rebind {
            typedef StlNodeAlloc<T2> other;
        };

        // #ifdef VERSION_SZBE69_B8
        // Retail doesn't have constructor calls
        StlNodeAlloc() {}
        StlNodeAlloc(StlNodeAlloc<T> const &) {}
        template <class T2>
        StlNodeAlloc(const StlNodeAlloc<T2> &) {}
        // #endif

        // ...but still has the destructor
        ~StlNodeAlloc() {}

        pointer address(reference value) const { return &value; }
        const_pointer address(const_reference value) const { return &value; }
        size_type max_size() const { return size_type(-1) / sizeof(T); }

        pointer allocate(const size_type count, const void *hint = nullptr) const {
            const char *name;
            if (gStlAllocNameLookup) {
                name = typeid(pointer).name();
            } else {
                name = gStlAllocName;
            }
            return reinterpret_cast<pointer>(
                MemOrPoolAllocSTL(count * sizeof(T), __FILE__, 0x39, name)
            );
        }

        void deallocate(pointer ptr, size_type count) const {
            int size = count * sizeof(T);
            const char *name;
            if (gStlAllocNameLookup) {
                name = typeid(pointer).name();
            } else {
                name = gStlAllocName;
            }
            MemOrPoolFreeSTL(size, ptr, __FILE__, 0x40, name);
        }

        void construct(pointer ptr, const_reference value) const { new (ptr) T(value); }
        void destroy(pointer ptr) const { ptr->~T(); }
    };

#ifdef STLPORT
}
#endif
