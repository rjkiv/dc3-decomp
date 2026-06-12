#include "Mesh.h"
#include "Rnd.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "os/System.h"
#include "rnddx9/Mat.h"
#include "rnddx9/Utl.h"
#include "rndobj/Fur.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/MeshVertCompress.h"
#include "rndobj/Rnd.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/VelocityBuffer.h"
#include "rndobj/Wind.h"
#include "utl/Std.h"
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

void DxMesh::DrawShowing() {
    DxMesh *owner = static_cast<DxMesh *>(mGeomOwner.Ptr());
    if (owner->CanDraw()) {
        if (owner->Verts().unkc) {
            Sync(0x1F);
        }
        if (TheRnd.DrawMode() == 6) {
            RndVelocityBuffer::Singleton().DrawMesh(this);
        } else {
            SetTransforms();
            for (RndMat *it = mMat; it != nullptr;) {
                if (it->Fur()) {
                    it = DrawFur(static_cast<DxMat *>(it));
                } else {
                    RndMat *next = it->NextPass();
                    ShaderType t = kStandardShader;
                    if (unk180 != kMaxShaderTypes) {
                        t = (ShaderType)unk180;
                    }
                    if (TheRnd.DrawMode() == 9) {
                        t = kAllWhiteShader;
                    }
                    RndShader::SelectConfig(it, t, false);
                    owner->DrawFacesInRange(0, -1);
                    it = next;
                }
            }
        }
    }
}

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

void DxMesh::OnSync(int flags) {
    PhysMemTypeTracker t("D3D(phys):Mesh");
    if (this != mGeomOwner) {
        if (Mutable() & 0x1F) {
            mGeomOwner->Sync(flags);
        }
        return;
    } else {
        RndMesh::OnSync(flags);
        if (mMutable) {
            return;
        } else {
            auto &verts = Verts();
            auto &faces = Faces();

            if (flags & 0x1FU) {
                unsigned int num = 0;
                unsigned int size = 0;
                bool b4 = false;
                mNumVerts = verts.size();
                if (mNumVerts != 0) {
                    num = mNumVerts;
                    size = VertSize();
                } else {
                    if (mNumCompressedVerts != 0) {
                        mNumVerts = mNumCompressedVerts;
                        num = mNumVerts;
                        size = VertSize();
                        b4 = true;
                    } else {
                        mVertexBufferData.Release();
                    }
                }
                if (!mVertexBufferData.mBuffer || mVertexBufferData.mSize != size * num) {
                    mVertexBufferData.Release();
                    if (num) {
                        D3DVertexBuffer *buffer =
                            MakeVertexBuffer(num, size, VertFVF(), false);
                        mVertexBufferData.SetData(buffer, size * num);
                    }
                }
                if (mVertexBufferData.mBuffer) {
                    if (b4) {
                        FillCompressedVerts();
                    } else {
                        Fill(verts.begin(), verts.end());
                    }
                }
            }

            if (flags & 0x20) {
                DX_RELEASE(unk1ac);
                mNumFaces = faces.size();
                if (mNumFaces) {
                    MILO_ASSERT(mNumFaces <= 0xFFFF, 0x17E);
                    unk1ac = MakeIndexBuffer(mNumFaces, 6, D3DFMT_INDEX16);
                    IBLock<RndMesh::Face> bLock(unk1ac, 0);
                    RndMesh::Face *faceItr = bLock.Data();
                    for (int i = 0; i < mNumFaces; i++) {
                        faceItr[i] = faces[i];
                    }
                }
            }

            if (!(flags & 0x200U)) {
                if (!(mMutable & 0x1F)) {
                    mVerts.resize(0);
                    ClearCompressedVerts();
                }
                if (!(mMutable & 0x20)) {
                    mFaces.swap(std::vector<Face>());
                }
            }
        }
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

D3DVertexBuffer *DxMesh::GetMultimeshFaces() {
    MILO_ASSERT(!Mutable(), 0x1A7);
    if (unk1b0) {
        return unk1b0;
    } else {
        unsigned int faces = mNumFaces * 3;
        TheDxRnd.Device()->CreateVertexBuffer(faces * 4, 0, 0, 0, &unk1b0, nullptr);
        int *vertexData;
        unk1b0->Lock(0, 0, (void **)&vertexData, 0);
        unsigned short *indexData;
        unk1ac->Lock(0, 0, (void **)&indexData, 0x10);
        while (faces-- != 0) {
            *vertexData++ = *indexData++;
        }
        unk1ac->Unlock();
        unk1b0->Unlock();
        return unk1b0;
    }
}

bool DxMesh::CheckFurTransformCache() {
    int i4 = NumBones();
    if (i4 == 0) {
        i4 = 1;
    }
    if (i4 != mTransformCache.size()) {
        mTransformCache.resize(i4);
        for (int i = 0; i < i4; i++) {
            mTransformCache[i].Reset();
        }
        return true;
    } else {
        return false;
    }
}

float DxMesh::FurWeight(RndMat *mat) {
    for (RndMat *it = mat; it != nullptr; it = it->NextPass()) {
        if (it->Fur()) {
            if (CheckFurTransformCache()) {
                return 1;
            }
            return 1.0f / (it->Fur()->Fluidity() * 6.5f + 1.0f);
        }
    }
    return -1;
}

void DxMesh::CacheFurTransform(const Transform &xfm, int i, float f3) {
    MILO_ASSERT(mTransformCache.size() > i, 0x1EE);
    Transform &cur = mTransformCache[i];
    Vector3 diff;
    Subtract(cur.v, xfm.v, diff);
    if (Dot(cur.m.y, xfm.m.y) >= 0.8660254f && LengthSquared(diff) < 2500.0f) {
        cur.m.x *= 1.0f - f3;
        cur.m.y *= 1.0f - f3;
        cur.m.z *= 1.0f - f3;
        cur.v *= 1.0f - f3;
        ScaleAddEq(cur, xfm, f3);
    } else {
        cur.Set(xfm.m, xfm.v);
    }

    RndFur *fur = mMat->Fur();
    if (fur->Wind()) {
        Vector3 v2;
        fur->Wind()->GetWind(xfm.v, TheTaskMgr.Seconds(TaskMgr::kRealTime), v2);
        ScaleAddEq(cur.v, v2, 0.05f);
    }
}

void DxMesh::SetTransforms() {
    bool oldShouldCache = mMotionCache.mShouldCache;
    mMotionCache.mShouldCache = false;
    unsigned int i4 = NumBones();
    TheShaderMgr.SetMeshInfo(i4, HasAOCalc());
    float weight = FurWeight(mMat);
    bool hasWeight = weight > 0;
    if (i4 == 0) {
        TheShaderMgr.UpdateCache(WorldXfm(), 0);
        if (hasWeight) {
            CacheFurTransform(WorldXfm(), 0, weight);
        }
        i4 = 1;
    } else {
        int idx = 0;
        FOREACH (it, mBones) {
            Transform tf90;
            Multiply(it->mOffset, it->mBone->WorldXfm(), tf90);
            TheShaderMgr.UpdateCache(tf90, idx);
            if (hasWeight) {
                CacheFurTransform(tf90, idx, weight);
            }
            ++idx;
        }
        TheNgStats->mBones += idx - 1;
        if (i4 < 1) {
            i4 = 1;
        }
    }
    TheShaderMgr.SetVConstant((VShaderConstant)0x5C, TheShaderMgr.ConstantCache(), i4 * 3);
    if (oldShouldCache) {
        RndVelocityBuffer::Singleton().CacheTransform(
            this, TheShaderMgr.ConstantCache(), i4
        );
    }
}

DxMat *DxMesh::DrawFur(DxMat *mat) {
    if (TheRnd.DrawMode() != 0) {
        return static_cast<DxMat *>(mat->NextPass());
    } else {
        DxMesh *owner = static_cast<DxMesh *>(mGeomOwner.Ptr());
        MILO_ASSERT(owner && owner->CanDraw(), 0x21B);
        MILO_ASSERT(mat, 0x21D);
        if (NumBones() * 2 >= 0x2B) {
            MILO_NOTIFY_ONCE(
                "%s: Too many bones for fur (%d > %d)", PathName(this), NumBones(), 0x15
            );
            return static_cast<DxMat *>(mat->NextPass());
        } else {
            RndFur *fur = mat->Fur();
            MILO_ASSERT(fur, 0x227);
            unsigned int numBones = NumBones();
            if (numBones == 0) {
                numBones = 1;
            }
            MILO_ASSERT(mTransformCache.size() == numBones, 0x22A);
            int vsc = 0x5C;
            for (int i = 0; i < numBones; i++, vsc += 3) {
                TheShaderMgr.SetVConstant4x3(
                    (VShaderConstant)vsc, Hmx::Matrix4(mTransformCache[i])
                );
            }
            fur->Prep(this, mat);

            static float sDefaultBias = -1;
            DWORD mipmapBias12, mipmapBias0;
            TheDxRnd.Device()->GetSamplerState(12, D3DSAMP_MIPMAPLODBIAS, &mipmapBias12);
            TheDxRnd.Device()->GetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, &mipmapBias0);
            TheDxRnd.Device()->SetSamplerState(
                12, D3DSAMP_MIPMAPLODBIAS, *reinterpret_cast<DWORD *>(&sDefaultBias)
            );
            TheDxRnd.Device()->SetSamplerState(
                0, D3DSAMP_MIPMAPLODBIAS, *reinterpret_cast<DWORD *>(&sDefaultBias)
            );
            int numPasses = fur->NumPasses();
            MILO_ASSERT(numPasses > 0, 0x243);
            DxMat *ret = static_cast<DxMat *>(mat->NextPass());
            for (int i = 0; i < numPasses; i++) {
                fur->Shell(i, this, mat);
                DrawFacesInRange(0, -1);
            }
            TheDxRnd.Device()->SetSamplerState(12, D3DSAMP_MIPMAPLODBIAS, mipmapBias12);
            TheDxRnd.Device()->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, mipmapBias0);
            TheDxRnd.Device()->SetSamplerState(6, D3DSAMP_MIPMAPLODBIAS, mipmapBias0);
            return ret;
        }
    }
}
