#include "os/HDCache.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "utl/FileStream.h"
#include "xdk/xapilibi/getcurrentthreadid.h"

HDCache TheHDCache;

HDCache::HDCache()
    : unk0(0), unk14(0), unk18(0), unk1c(0), unk20(-1), unk24(0), unk2c(0), unk30(-1),
      unk34(-1), mLockId(-1), unk3c(0), unk40(0), unk44(0), unk50(0), unk64(0) {}

HDCache::~HDCache() {}

bool HDCache::LockCache() {
    CritSecTracker cst(unk40);

    if (mLockId == -1 || mLockId == GetCurrentThreadId()) {
        mLockId = GetCurrentThreadId();
        unk3c++;
        return true;
    } else {
        return false;
    }
}

void HDCache::UnlockCache() {
    CritSecTracker cst(unk40);
    MILO_ASSERT(mLockId == GetCurrentThreadId(), 0xfa);
    unk3c--;
    if (unk3c == 0)
        unk3c = -1;
}

bool HDCache::ReadFail() { return false; }

bool HDCache::ReadDone() { return false; }

bool HDCache::WriteDone() { return false; }

void HDCache::Poll() {}

bool HDCache::ReadAsync(int, int, void *) { return false; }

bool HDCache::WriteAsync(int, int, void const *) { return false; }

void HDCache::Init() {}

int HDCache::HdrSize() { return 1; }

FileStream *HDCache::OpenHeader() { return 0; }

void HDCache::WriteHdr() {}

void HDCache::OpenFiles(int) {}
