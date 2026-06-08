#pragma once

#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include <xdk/D3D9.h>

template <typename T>
class BufLock {
public:
    BufLock(T *buf, uint flags) : mBuf(buf) {
        MILO_ASSERT(mBuf, 27);
        if (mBuf->IsSet(TheDxRnd.Device())) {
            TheDxRnd.Device()->SetStreamSource(0, nullptr, 0, 0);
            TheDxRnd.Device()->SetStreamSource(1, nullptr, 0, 0);
        }
        mBuf->Lock(0, 0, &mDataAddr, flags);
    }
    virtual ~BufLock() { mBuf->Unlock(); }

protected:
    T *mBuf;
    void *mDataAddr;
};

template <typename T = void>
class VBLock : public BufLock<D3DVertexBuffer> {
public:
    VBLock(D3DVertexBuffer *buf, uint flags) : BufLock(buf, flags) {}
    virtual ~VBLock() {}

    T *Data() const { return (T *)mDataAddr; }
};

template <typename T = void>
class IBLock : public BufLock<D3DIndexBuffer> {
public:
    IBLock(D3DIndexBuffer *buf, uint flags) : BufLock(buf, flags) {}
    virtual ~IBLock() {}

    T *Data() const { return (T *)mDataAddr; }
};

struct D3DVertexBuffer *MakeVertexBuffer(int, uint, uint, bool);
struct D3DIndexBuffer *MakeIndexBuffer(int, uint, D3DFORMAT);
struct D3DVertexBuffer *CloneVertexBuffer(struct D3DVertexBuffer *);
struct D3DIndexBuffer *CloneIndexBuffer(struct D3DIndexBuffer *);
