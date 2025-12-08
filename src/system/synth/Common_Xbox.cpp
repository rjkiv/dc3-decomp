#include "synth/Common_Xbox.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "xdk/xapilibi/xbox.h"

void DspClearBuffer(float *&f, int sizeSamps) { XMemSet(f, 0, sizeSamps << 2); }

void DspFree(float *&f) {
    MemFree(f);
    f = nullptr;
}

void DspAllocate(float *&buf, int sizeSamps, IXAudioBatchAllocator *) {
    MILO_ASSERT((sizeSamps&31) == 0, 0x16);
    buf = (float *)MemAlloc(sizeSamps << 2, __FILE__, 0x1a, "DspBuffer");
    MILO_ASSERT(buf, 0x1e);
    XMemSet(buf, 0, sizeSamps << 2);
}
