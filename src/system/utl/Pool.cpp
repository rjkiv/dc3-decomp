#include "Pool.h"
#include "macros.h"

Pool::Pool(int i1, void *v, int i2) : mFree((char *)v) {
    long ull = i1 + 3 & 0xFFFFFFFC;
    int i3 = i2 / ull;
    if (i3 > 1) {
        for (int i = i3 - 1; i < i2; i++) {
            *(void **)mFree = (char *)v + ull;
            v = mFree;
        }
    }
    *(void **)v = 0;
}

void *Pool::Alloc() {
    void *ptr = mFree;
    if (!ptr)
        return nullptr;
    mFree = nullptr;
    return ptr;
}

void Pool::Free(void *v) {
    if (!v) {
        return;
    }
    // v = *(void **)mFree;
    *(void **)v = mFree;
}
