#include "synth_xbox/ExternalMic.h"
#include "os/Debug.h"
#include "xdk/XAPILIB.h"

namespace {
    DWORD ExternalMicThreadEntry(void *v) {
        ExternalMic *mic = (ExternalMic *)v;
        return mic->sampleProcessThread();
    }

    std::vector<ExternalMic *> gMics;
}

ExternalMic::ExternalMic(unsigned long ul)
    : unk4(ul), unk8(false), unk9(false), unkc(-1.0f) {
    mThread = CreateThread(0, 0, ExternalMicThreadEntry, this, 4, 0);
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
