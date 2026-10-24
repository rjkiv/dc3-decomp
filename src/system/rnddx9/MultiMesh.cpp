#include "rnddx9/MultiMesh.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "rnddx9/Mesh.h"
#include "rnddx9/Rnd.h"
#include "rnddx9/Utl.h"
#include "rndobj/Mesh.h"
#include "rndobj/MeshVertCompress.h"
#include "rndobj/Rnd.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Spline.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Trans.h"
#include "xdk/D3D9.h"

DxMultiMesh::DxMultiMesh() : mLastMat(0), mFrame(0) {
    for (int i = 0; i < 3; i++) {
        mMutableVertBuffer[i] = mMutableIndexBuffer[i] = nullptr;
    }
}

DxMultiMesh::~DxMultiMesh() {
    for (int i = 0; i < 3; i++) {
        DX_RELEASE(mMutableIndexBuffer[i]);
        DX_RELEASE(mMutableVertBuffer[i]);
    }
}

void DxMultiMesh::DrawShowing() {
    if (!mInstances.empty() && mMesh) {
        if (mMesh->NumBones() != 0) {
            MILO_LOG("MultiMesh: mesh can't be skinned\n");
        } else {
            DxMesh *mesh = static_cast<DxMesh *>(mMesh.Ptr());
            if (mesh->OwnerCanDraw() && TheRnd.DrawMode() != Rnd::kDrawOcclusion
                && TheRnd.DrawMode() == Rnd::kDrawNormal) {
                DrawBatchedNewGfx();
            }
        }
    }
}

static D3DVERTEXELEMENT9 sVertexElement[] = {
    { 0,
      offsetof(CompressedVertex_Xbox, mPosX),
      D3DDECLTYPE_FLOAT3,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_POSITION,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mColor),
      D3DDECLTYPE_D3DCOLOR,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_COLOR,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mTex),
      D3DDECLTYPE_FLOAT16_2,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TEXCOORD,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mNormal),
      D3DDECLTYPE_DEC4N,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_NORMAL,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mTangent),
      D3DDECLTYPE_DEC4N,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TANGENT,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mWeight),
      D3DDECLTYPE_UDEC4N,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_BLENDWEIGHT,
      0 },
    { 0,
      offsetof(CompressedVertex_Xbox, mIndices),
      D3DDECLTYPE_UBYTE4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_BLENDINDICES,
      0 },
    { 1, 0, D3DDECLTYPE_UINT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
    D3DDECL_END()
};

static D3DVERTEXELEMENT9 sMutableVertexElement[] = {
    { 0,
      offsetof(RndMesh::Vert, pos),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_POSITION,
      0 },
    { 0,
      offsetof(RndMesh::Vert, norm),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_NORMAL,
      0 },
    { 0,
      offsetof(RndMesh::Vert, boneWeights),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_BLENDWEIGHT,
      0 },
    { 0,
      offsetof(RndMesh::Vert, color),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_COLOR,
      0 },
    { 0,
      offsetof(RndMesh::Vert, tex),
      D3DDECLTYPE_FLOAT2,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TEXCOORD,
      0 },
    { 0,
      offsetof(RndMesh::Vert, bone0),
      D3DDECLTYPE_SHORT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_BLENDINDICES,
      0 },
    { 0,
      offsetof(RndMesh::Vert, tangent),
      D3DDECLTYPE_FLOAT4,
      D3DDECLMETHOD_DEFAULT,
      D3DDECLUSAGE_TANGENT,
      0 },
    { 1, 0, D3DDECLTYPE_UINT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
    D3DDECL_END()
};

void DxMultiMesh::Init() {
    REGISTER_OBJ_FACTORY(DxMultiMesh);
    HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(sVertexElement, &sVertexDecl);
    DX_ASSERT(hr, 0x97);
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
    DxMesh *owner = static_cast<DxMesh *>(mMesh->GeomOwner());
    MILO_ASSERT(!owner->IsSkinned(), 538);
    MILO_ASSERT(owner->Mutable(), 539);
    int idx = mFrame % 3;
    if (!mMutableIndexBuffer[idx]) {
        HRESULT hr = TheDxRnd.Device()->CreateVertexBuffer(
            owner->RndMesh::NumVerts(),
            0,
            owner->VertFVF(),
            0,
            &mMutableIndexBuffer[idx],
            nullptr
        );
        DX_ASSERT(hr, 0x225);
    }
    {
        VBLock<RndMesh::Vert> vbLock(mMutableIndexBuffer[idx], 0);
        memcpy(
            vbLock.Data(),
            owner->Verts().begin(),
            owner->Verts().size() * sizeof(RndMesh::Vert)
        );
    }
}

void DxMultiMesh::DrawBatchedNewGfx() {
    if (mInstances.size() == 0)
        return;
    DxMesh *owner = static_cast<DxMesh *>(mMesh->GeomOwner());
    bool fastbillboard = owner->TransConstraint() == RndTransformable::kFastBillboardXYZ;
    MILO_ASSERT(!owner->IsSkinned(), 593);
    if (owner->Mutable()) {
        UpdateGeometryBuffers();
    }
    int face_count;
    if (owner->Mutable()) {
        TheDxRnd->SetStreamSource(
            0, mMutableVertBuffer[mFrame % 3u], 0, sizeof(RndMesh::Vert)
        );
        TheDxRnd->SetStreamSource(1, mMutableIndexBuffer[mFrame % 3u], 0, 4);
        TheDxRnd->SetVertexDeclaration(sMutableVertexDecl);
        face_count = owner->Faces().size();
    } else {
        TheDxRnd->SetStreamSource(0, owner->mVBVerts.GetBuffer(), 0, owner->VertSize());
        TheDxRnd->SetStreamSource(1, owner->GetMultimeshFaces(), 0, 4);
        TheDxRnd->SetVertexDeclaration(sVertexDecl);
        face_count = owner->mNumFaces;
    }
    face_count *= 3;
    TheShaderMgr.SetVConstant(
        kVShader_MultiMeshIndices, Vector4(face_count, face_count, face_count, face_count)
    );
    RndMat *mtl = owner->Mat();
    int inst_xfm_slot_stop =
        RndSpline::GlobalDefaultSpline() == nullptr ? kVShader_TransformEndSlot : 173;
    do {
        int inst_ct = 0;
        int batch_ct = 0;
        TheShaderMgr.SetTransform(Transform::GetIdentity());
        RndShader::SelectConfig(
            mtl, fastbillboard ? kMultimeshBBShader : kMultimeshShader, 0
        );
        FOREACH (it, mInstances) {
            int instidx = 0;
            for (int tlist = kVShader_TransformList0; tlist < inst_xfm_slot_stop;
                 tlist += kVShader_TransformNumSlots) {
                if (it != mInstances.end()) {
                    auto &instance = *it;
                    if (instance.mShowing) {
                        TheShaderMgr.SetVConstant4x3(
                            static_cast<VShaderConstant>(tlist), instance.mXfm
                        );
                        instidx++;
                    }
                }
            }
            if (instidx <= 0)
                continue;
            TheDxRnd->DrawVertices(D3DPT_TRIANGLELIST, 0, face_count * instidx);
            inst_ct += instidx;
            batch_ct++;
        }
        if (mtl) {
            mtl = mtl->NextPass();
        }
        TheNgStats->mMultiMeshInsts += inst_ct;
        TheNgStats->mMultiMeshBatches += batch_ct;
        TheNgStats->mFaces += inst_ct * owner->NumFaces();
    } while (mtl != nullptr);
    mFrame++;
}
