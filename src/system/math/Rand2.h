#pragma once
#include "utl/MemMgr.h"

/** Used for decrypting encrypted BinStreams */
class Rand2 {
private:
    int mSeed;

public:
    Rand2(int);
    int Int();

    static void operator delete(void *ptr) {
        MemFree(ptr, "e:\\lazer_build_gmc1\\system\\src\\math/Rand2.h", 18, __FUNCTION__);
    }
};
