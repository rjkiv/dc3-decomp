#include "synth/Mic.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "obj/Data.h"
#include "utl/MemMgr.h"
#include <cstring>

void Mic::Set(const DataArray *data) {
    MILO_ASSERT(data, 0x12);
    SetGain(data->FindFloat("gain"));
    SetDMA(data->FindInt("dma"));
    DataArray *compressorArr = data->FindArray("compressor");
    SetCompressor(compressorArr->Int(1));
    SetCompressorParam(compressorArr->Float(2));
}

RingBuffer::~RingBuffer() {
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x23);
        mBuffer = nullptr;
    }
}

void RingBuffer::Reset() {
    memset(mBuffer, 0, mSize);
    mWriteIx = 0;
    mReadIx = 0;
    mTotal = 0;
}

void RingBuffer::Init(int size) {
    mSize = size;
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x2B);
        mBuffer = nullptr;
    }
    mBuffer = MemAlloc(size, __FILE__, 0x2C, "VirtualMic RingBuffer", 0x80);
    MILO_ASSERT(mBuffer, 0x2D);
    Reset();
}

int RingBuffer::Peek(void *data, int len) {
    MILO_ASSERT(len <= mSize, 0x62);
    int i2 = ((mWriteIx - len) + mSize) % mSize;
    int val = std::min(mSize - i2, len);
    memcpy(data, (char *)mBuffer + i2, val);
    if (val != len) {
        memcpy((char *)data + val, mBuffer, len - val);
    }
    return len;
}

int RingBuffer::Write(void *data, int len) {
    if (len > mSize) {
        data = (char *)data + (len - mSize);
        len = mSize;
    }

    int i40 = mSize - mWriteIx;
    int ret = (mTotal - mSize) + len;
    int _size = std::min(i40, len);

    memcpy((char *)mBuffer + mWriteIx, data, _size);
    if (_size != len) {
        memcpy(mBuffer, (char *)data + _size, len - _size);
    }

    mWriteIx = (mWriteIx + len) % mSize;
    mTotal = std::min(mSize, mTotal + len);
    if (mTotal == mSize) {
        mReadIx = mWriteIx;
    }
    return ret;
}

int RingBuffer::Read(void *data, int len) {
    int i4 = mTotal < len ? mTotal : len;
    if (i4 == 0) {
        return 0;
    } else {
        int _size = std::min(mSize - mReadIx, i4);
        memcpy(data, (char *)mBuffer + mReadIx, _size);
        if (_size != i4) {
            memcpy((char *)data + _size, mBuffer, i4 - _size);
        }
        mTotal -= i4;
        mReadIx = (mReadIx + i4) % mSize;
        return i4;
    }
}
