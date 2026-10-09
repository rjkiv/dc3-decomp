#include "HolmesClient.h"
#include "os/AsyncFileHolmes_p.h"
#include "os/Debug.h"

AsyncFileHolmes::AsyncFileHolmes(const char *filename, int mode)
    : AsyncFile(filename, mode), mFileHandle(-1) {}

AsyncFileHolmes::~AsyncFileHolmes() { Terminate(); }

bool AsyncFileHolmes::Truncate(int length) {
    HolmesClientTruncate(mFileHandle, length);
    return true;
}

void AsyncFileHolmes::_OpenAsync() {
    unsigned int siz;
    mFail = !HolmesClientOpen(mFilename.c_str(), mMode, siz, mFileHandle);
    if (mFail) {
        siz = 0;
    }
    mSize = siz;
}

void AsyncFileHolmes::_WriteAsync(const void *buf, int bytes) {
    MILO_ASSERT(mOffset == bytes, 0x26);
    HolmesClientWrite(mFileHandle, mTell - mOffset, bytes, buf);
}

void AsyncFileHolmes::_SeekToTell() {
    while (!_ReadDone())
        ;
}

void AsyncFileHolmes::_ReadAsync(void *buf, int bytes) {
    HolmesClientRead(mFileHandle, mTell, bytes, buf, this);
}

bool AsyncFileHolmes::_ReadDone() { return HolmesClientReadDone(this); }

void AsyncFileHolmes::_Close() {
    if (!mFail) {
        HolmesClientClose(this, mFileHandle);
    }
}
