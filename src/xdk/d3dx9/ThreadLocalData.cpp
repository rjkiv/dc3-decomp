#include "threadlocaldata.h"
#include "types.h"
#include "xapilibi/processthreadsapi.h"

static uint s_tlsIndex = -1;
static volatile int g_NumContenders;
static volatile int g_SpinLock;

namespace D3DXShader {
    ThreadLocalData *GetThreadLocalData() {
        uint idx = s_tlsIndex;
        ThreadLocalData *p = nullptr;
        if (idx != -1)
            p = reinterpret_cast<ThreadLocalData *>(TlsGetValue(idx));
        return p;
    }
}
