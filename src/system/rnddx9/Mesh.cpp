#include "Mesh.h"
#include "Rnd.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "os/System.h"
#include "rnddx9/Utl.h"
#include "rndobj/Mesh.h"
#include "rndobj/MeshVertCompress.h"
#include "rndobj/Stats_NG.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

void DxMesh::VertexBufferData::SetData(D3DVertexBuffer *buffer, unsigned int size) {
    MILO_ASSERT(buffer != NULL, 0x1E);
    MILO_ASSERT(size > 0, 0x1F);
    mBuffer = buffer;
    mSize = size;
}

void DxMesh::VertexBufferData::Release() {
    DX_RELEASE(mBuffer);
    mSize = 0;
}

DxMesh::DxMesh() : mNumVerts(0), mNumFaces(0), unk1ac(0), unk1b0(0) {
    // clang-format off
    static const D3DVERTEXELEMENT9 sVertexElements[] = {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT16_2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 20, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 24, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        { 0, 28, D3DDECLTYPE_UDEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
        { 0, 32, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        D3DDECL_END(),
        { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 72, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        D3DDECL_END(),
        // why is there a random 4 byte gap here. whyyyyyyyy
        { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
        { 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 72, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        D3DDECL_END()
    };
    // clang-format on
    if (!sVertexDecl) {
        HRESULT hr =
            TheDxRnd.Device()->CreateVertexDeclaration(sVertexElements, &sVertexDecl);
        DX_ASSERT(hr, 0xA8);
    }
    if (!sMutableVertexDecl) {
        HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(
            &sVertexElements[8], &sMutableVertexDecl
        );
        DX_ASSERT(hr, 0xAF);
    }
    if (!sMutableSkinnedVertexDecl) {
        HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(
            &sVertexElements[15], &sMutableSkinnedVertexDecl
        );
        DX_ASSERT(hr, 0xB5);
    }
}

DxMesh::~DxMesh() {
    DX_RELEASE(unk1ac);
    DX_RELEASE(unk1b0);
}

BEGIN_COPYS(DxMesh)
    COPY_SUPERCLASS(RndMesh)
    CREATE_COPY(DxMesh)
    if (c && this == mGeomOwner && mMutable == 0) {
        PhysMemTypeTracker t("D3D(phys):Mesh");
        mVertexBufferData.Release();
        COPY_MEMBER(mNumVerts)
        if (mNumVerts != 0) {
            D3DVertexBuffer *buffer = CloneVertexBuffer(c->mVertexBufferData.mBuffer);
            mVertexBufferData.SetData(buffer, c->mVertexBufferData.mSize);
        }
        DX_RELEASE(unk1ac);
        COPY_MEMBER(mNumFaces)
        if (mNumFaces != 0) {
            unk1ac = CloneIndexBuffer(c->unk1ac);
        }
    }
END_COPYS

void DxMesh::DrawFacesInRange(int x, int y) {
    D3DDevice *device = TheDxRnd.Device();
    if (mMutable) {
        if (!Faces().empty()) {
            TheNgStats->mMutMeshes++;
            device->SetVertexDeclaration(
                IsSkinned() ? sMutableSkinnedVertexDecl : sMutableVertexDecl
            );
            void *faceData = nullptr;
            void *vertData = nullptr;
            HRESULT hr = device->BeginIndexedVertices(
                D3DPT_TRIANGLELIST,
                0,
                Verts().size(),
                Faces().size() * 3,
                D3DFMT_INDEX16,
                0x60, // sizeof(Vert)?
                &faceData,
                &vertData
            );
            DX_ASSERT(hr, 0x35C);
            XMemCpyStreaming_WriteCombined(
                faceData, &Faces(0), Faces().size() * sizeof(Face)
            );
            XMemCpyStreaming_WriteCombined(
                vertData, &Verts(0), Verts().size() * sizeof(Vert)
            );
            device->EndIndexedVertices();
            TheNgStats->mFaces += Faces().size();
        }
    } else {
        if (y == -1) {
            y = mNumFaces;
        }
        device->SetIndices(unk1ac);
        D3DVertexBuffer *buffer = mVertexBufferData.mBuffer;
        device->SetStreamSource(0, buffer, 0, VertSize());
        device->SetVertexDeclaration(sVertexDecl);
        TheNgStats->mRegMeshes++;
        TheNgStats->mFaces += y;
        if (mNumFaces == 0) {
            MILO_NOTIFY_ONCE(
                "%s (%s): Trying to draw mesh with no faces", Name(), PathName(this)
            );
        } else {
            device->DrawIndexedVertices(D3DPT_TRIANGLELIST, 0, x * 3, y * 3);
        }
        device->SetIndices(nullptr);
    }
}

void DxMesh::Fill(RndMesh::Vert *v1, RndMesh::Vert *v2) {
    VBLock<CompressedVertex_Xbox> lock(mVertexBufferData.mBuffer, 0);
    if (v1 != v2) {
        CompressedVertex_Xbox *xboxIt = lock.Data();
        for (; v1 != v2; ++v1, ++xboxIt) {
            FillCompressedVertex(*xboxIt, *v1, false);
        }
    }
}

void DxMesh::FillCompressedVerts() {
    MILO_ASSERT(mNumCompressedVerts > 0, 0x115);
    MILO_ASSERT(mCompressedVerts != NULL, 0x116);
    VBLock<CompressedVertex_Xbox> lock(mVertexBufferData.mBuffer, 0);
    memcpy(lock.Data(), mCompressedVerts, mNumCompressedVerts * VertSize());
}

unsigned int DxMesh::VertSize() const {
    if (GetGfxMode() == kNewGfx) {
        return 0x24;
    } else {
        return IsSkinned() ? 0x30 : 0x24;
    }
}

unsigned int DxMesh::VertFVF() const {
    if (GetGfxMode() == kNewGfx) {
        return 0;
    } else {
        return IsSkinned() ? 0x61 : 0x152;
    }
}

bool DxMesh::CanDraw() const {
    D3DVertexBuffer *buf = mVertexBufferData.mBuffer;
    bool lmao = buf && unk1ac;
    return lmao || mMutable;
}

void _fake(void) {
    BufLock<struct D3DVertexBuffer> buf(nullptr, 0);
    BufLock<struct D3DIndexBuffer> buf2(nullptr, 0);
}
