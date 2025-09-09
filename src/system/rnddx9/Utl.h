#pragma once

#include "os/Debug.h"
#include "rnddx9/Rnd_Xbox.h"
#include "xdk/d3d9i/resource.h"
#include <xdk/D3D9.h>

template <typename T>
class BufLock {
public:
    BufLock(T *buf, uint flags) : mBuf(buf) {
        MILO_ASSERT(mBuf, 27);
        if (D3DResource_IsSet(
                reinterpret_cast<D3DResource *>(mBuf), TheDxRnd.D3DDevice()
            )) {
            D3DDevice_SetStreamSource(TheDxRnd.D3DDevice(), 0, nullptr, 0, 0, 1);
            D3DDevice_SetStreamSource(TheDxRnd.D3DDevice(), 1, nullptr, 0, 0, 1);
        }
        mDataAddr = // this is SUPPOSED to be a COM lookup, but idk how those work or
                    // any hacks to make this work anyways
            (void *)D3DVertexBuffer_Lock(
                reinterpret_cast<D3DVertexBuffer *>(mBuf), 0, 0, flags
            );
    }
    virtual ~BufLock() {
        D3DVertexBuffer_Unlock(reinterpret_cast<D3DVertexBuffer *>(mBuf));
    }

    T *mBuf;
    void *mDataAddr;
};

template <typename T = void>
class VBLock : public BufLock<D3DVertexBuffer> {
public:
    VBLock(D3DVertexBuffer *buf, uint flags) : BufLock(buf, flags) {}
    virtual ~VBLock() {}
};

template <typename T = void>
class IBLock : public BufLock<D3DIndexBuffer> {
public:
    IBLock(D3DIndexBuffer *buf, uint flags) : BufLock(buf, flags) {}
    virtual ~IBLock() {}
};

struct D3DVertexBuffer *MakeVertexBuffer(int, uint, uint, bool);
struct D3DIndexBuffer *MakeIndexBuffer(int, uint, D3DFORMAT);
struct D3DVertexBuffer *CloneVertexBuffer(struct D3DVertexBuffer *);
struct D3DIndexBuffer *CloneIndexBuffer(struct D3DIndexBuffer *);
