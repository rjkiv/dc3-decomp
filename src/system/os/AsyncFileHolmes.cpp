#include "HolmesClient.h"
#include "os/AsyncFileHolmes_p.h"
#include "os/Debug.h"

AsyncFileHolmes::AsyncFileHolmes(const char *name, int mode)
    : AsyncFile(name, mode), unk34(-1) {}

AsyncFileHolmes::~AsyncFileHolmes() { Terminate(); }

bool AsyncFileHolmes::Truncate(int x) {
    HolmesClientTruncate(unk34, x);
    return true;
}

void AsyncFileHolmes::_OpenAsync() {
    unsigned int siz;
    mFail = !HolmesClientOpen(mFilename.c_str(), mMode, siz, unk34);
    if (mFail) {
        siz = 0;
    }
    mSize = siz;
}

void AsyncFileHolmes::_WriteAsync(const void *data, int bytes) {
    MILO_ASSERT(mOffset == bytes, 0x26);
    HolmesClientWrite(unk34, mTell - mOffset, bytes, data);
}

void AsyncFileHolmes::_SeekToTell() {
    while (!_ReadDone())
        ;
}

void AsyncFileHolmes::_ReadAsync(void *data, int bytes) {
    HolmesClientRead(unk34, mTell, bytes, data, this);
}

bool AsyncFileHolmes::_ReadDone() { return HolmesClientReadDone(this); }

void AsyncFileHolmes::_Close() {
    if (!mFail) {
        HolmesClientClose(this, unk34);
    }
}
