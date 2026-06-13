#include "rnddx9/MultiMesh.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "rnddx9/Mesh.h"
#include "rnddx9/Rnd.h"
#include "rnddx9/Utl.h"
#include "rndobj/Mesh.h"
#include "rndobj/Rnd.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

DxMultiMesh::DxMultiMesh() : unk5c(0), unk60(0) {
    for (int i = 0; i < 3; i++) {
        unk64[i] = unk70[i] = nullptr;
    }
}

DxMultiMesh::~DxMultiMesh() {
    for (int i = 0; i < 3; i++) {
        DX_RELEASE(unk70[i]);
        DX_RELEASE(unk64[i]);
    }
}

void DxMultiMesh::DrawShowing() {
    if (!mInstances.empty() && mMesh) {
        if (mMesh->NumBones() != 0) {
            MILO_LOG("MultiMesh: mesh can't be skinned\n");
        } else {
            DxMesh *mesh = static_cast<DxMesh *>(mMesh.Ptr());
            if (mesh->OwnerCanDraw() && TheRnd.DrawMode() != 5
                && TheRnd.DrawMode() == 0) {
                DrawBatchedNewGfx();
            }
        }
    }
}

void DxMultiMesh::Init() {
    REGISTER_OBJ_FACTORY(DxMultiMesh);
    static D3DVERTEXELEMENT9 sVertexElement[] = {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT16_2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 20, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 24, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        { 0, 28, D3DDECLTYPE_UDEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
        { 0, 32, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        { 1, 0, D3DDECLTYPE_UINT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
        D3DDECL_END()
    };
    HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(sVertexElement, &sVertexDecl);
    DX_ASSERT(hr, 0x97);
    static D3DVERTEXELEMENT9 sMutableVertexElement[] = {
        { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
        { 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 72, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
        { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
        { 1, 0, D3DDECLTYPE_UINT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
        D3DDECL_END()
    };
    hr = TheDxRnd.Device()->CreateVertexDeclaration(
        sMutableVertexElement, &sMutableVertexDecl
    );
    DX_ASSERT(hr, 0x9A);
}

void DxMultiMesh::Shutdown() {
    if (sVertexDecl) {
        sVertexDecl->Release();
        sVertexDecl = nullptr;
    }
    if (sMutableVertexDecl) {
        sMutableVertexDecl->Release();
        sMutableVertexDecl = nullptr;
    }
}

void DxMultiMesh::UpdateGeometryBuffers() {
    PhysMemTypeTracker t("D3D(phys):Mesh");
    DxMesh *owner = static_cast<DxMesh *>(mMesh->GetGeomOwner());
    MILO_ASSERT(!owner->IsSkinned(), 0x21A);
    MILO_ASSERT(owner->Mutable(), 0x21B);
    int idx = unk60 % 3;
    if (!unk70[idx]) {
        HRESULT hr = TheDxRnd.Device()->CreateVertexBuffer(
            owner->RndMesh::NumVerts(), 0, owner->VertFVF(), 0, &unk70[idx], nullptr
        );
        DX_ASSERT(hr, 0x225);
    }
    {
        VBLock<RndMesh::Vert> vbLock(unk70[idx], 0);
        memcpy(
            vbLock.Data(),
            owner->Verts().begin(),
            owner->Verts().size() * sizeof(RndMesh::Vert)
        );
    }
}
