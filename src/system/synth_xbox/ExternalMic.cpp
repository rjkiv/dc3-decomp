#include "synth_xbox/ExternalMic.h"
#include "os/Debug.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbox.h"

namespace {
    unsigned long ExternalMicThreadEntry(void *v) { return 1; }
}

ExternalMic::ExternalMic(unsigned long ul)
    : unk4(ul), unk8(false), unk9(false), unkc(-1.0f) {
    mThread = CreateThread(
        0, 0, (LPTHREAD_START_ROUTINE(__cdecl *))ExternalMicThreadEntry(0), this, 4, 0
    );
    MILO_ASSERT(mThread, 0x6a);
    SetThreadPriority(mThread, 15);
    XSetThreadProcessor(mThread, 3);
    ResumeThread(mThread);
}

ExternalMic::~ExternalMic() {
    unk8 = true;
    WaitForSingleObject(mThread, -1);
    CloseHandle(mThread);
}
