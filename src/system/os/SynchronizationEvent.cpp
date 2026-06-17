#include "os/SynchronizationEvent.h"
#include "os/Debug.h"
#include "xdk/XAPILIB.h"

SynchronizationEvent::~SynchronizationEvent() { CloseHandle(mEvent); }

bool SynchronizationEvent::Wait(int i1) {
    int temp = i1;
    if (i1 == -1)
        temp = -1;
    return WaitForSingleObject(mEvent, temp) != 0x102;
}

SynchronizationEvent::SynchronizationEvent()
    : mEvent(CreateEventA(nullptr, 0, 0, nullptr)) {
    MILO_ASSERT(mEvent, 0x12);
}

void SynchronizationEvent::Set() {
    BOOL success = SetEvent(mEvent);
    MILO_ASSERT(success, 0x1E);
}
