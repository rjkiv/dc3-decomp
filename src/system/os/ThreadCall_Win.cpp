#include "ThreadCall.h"
#include "os/Debug.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/closehandle.h"
#include "xdk/xapilibi/getcurrentthreadid.h"
#include "xdk/xapilibi/waitforsingleobject.h"

namespace {
    bool gReadyForNext = true;
    bool gCallDone;
    bool gTerminate;
    HANDLE gThreadSema;
    HANDLE gThreadHandle;

    uint MyThreadFunc(void *) {
        WaitForSingleObject(gThreadSema, -1);
        while (!gTerminate) {
            MILO_ASSERT(false, 199);
            WaitForSingleObject(gThreadSema, -1);
        }
        CloseHandle(gThreadSema);
        return 0;
    }
}

u32 gMainThreadID = -1;

void ThreadCallInit() {}

void ThreadCall(ThreadCallFunc, ThreadCallCallbackFunc) {}

void ThreadCallPreInit() { gMainThreadID = GetCurrentThreadId(); }

void ThreadCallTerminate() {}
