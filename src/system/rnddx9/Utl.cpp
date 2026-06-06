#pragma once
#include "Utl.h"
#include "rnddx9/Rnd.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"
#include <cstring>

D3DVertexBuffer *MakeVertexBuffer(int num, uint size, uint, bool) {
    MILO_ASSERT(num > 0, 19);
    MILO_ASSERT(size != 0, 20);

    D3DVertexBuffer *vb;
    HRESULT hr = TheDxRnd.Device()->CreateVertexBuffer(
        num * size, 0, 0, D3DPOOL_DEFAULT, &vb, nullptr
    );
    DX_ASSERT_CODE(hr, 0x22);
    return vb;
}

D3DIndexBuffer *MakeIndexBuffer(int num, uint size, D3DFORMAT fmt) {
    MILO_ASSERT(num > 0, 60);
    MILO_ASSERT(size != 0, 61);
    MILO_ASSERT(fmt == D3DFMT_INDEX16 || fmt == D3DFMT_INDEX32, 62);

    D3DIndexBuffer *ib;
    HRESULT hr = TheDxRnd.Device()->CreateIndexBuffer(
        num * size, 8, fmt, D3DPOOL_MANAGED, &ib, nullptr
    );
    DX_ASSERT_CODE(hr, 0x47);
    return ib;
}

D3DVertexBuffer *CloneVertexBuffer(D3DVertexBuffer *in) {
    if (in == nullptr)
        return in;
    D3DVERTEXBUFFER_DESC desc;
    in->GetDesc(&desc);

    D3DVertexBuffer *out;
    HRESULT hr = TheDxRnd.Device()->CreateVertexBuffer(
        desc.Size, desc.Usage, 0, desc.Pool, &out, nullptr
    );
    DX_ASSERT_CODE(hr, 49);
    VBLock<> lock_in(in, 0);
    VBLock<> lock_out(out, 0);
    memcpy(lock_out.mDataAddr, lock_in.mDataAddr, desc.Size);
    return out;
}

D3DIndexBuffer *CloneIndexBuffer(D3DIndexBuffer *in) {
    if (in == nullptr)
        return in;
    D3DINDEXBUFFER_DESC desc;
    in->GetDesc(&desc);

    D3DIndexBuffer *out;
    HRESULT hr = TheDxRnd.Device()->CreateIndexBuffer(
        desc.Size, desc.Usage, desc.Format, desc.Pool, &out, nullptr
    );
    DX_ASSERT_CODE(hr, 86);
    IBLock<> lock_in(in, 0);
    IBLock<> lock_out(out, 0);
    memcpy(lock_out.mDataAddr, lock_in.mDataAddr, desc.Size);
    return out;
}
