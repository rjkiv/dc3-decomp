#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rnddx9/Mat.h"
#include "rnddx9/MultiMesh.h"
#include "rnddx9/Object.h"
#include "rndobj/Mesh.h"
#include "utl/PoolAlloc.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"

class DxMesh : public RndMesh, public DxObject {
    friend class DxMultiMesh;

public:
    struct VertexBufferData {
        VertexBufferData() : mBuffer(0), mSize(0) {}
        ~VertexBufferData() { Release(); }
        // void GetBuffer(D3DVertexBuffer *&) const;
        D3DVertexBuffer *GetBuffer() { return mBuffer; }
        const D3DVertexBuffer *GetBuffer() const { return mBuffer; }
        unsigned int GetSize() const { return mSize; }
        void SetData(D3DVertexBuffer *, unsigned int);
        void Release();

    private:
        D3DVertexBuffer *mBuffer; // 0x0
        unsigned int mSize; // 0x4
    };
    // Hmx::Object
    virtual ~DxMesh();
    OBJ_CLASSNAME(Mesh)
    OBJ_SET_TYPE(Mesh)
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    // RndMesh
    virtual void DrawShowing();
    virtual void DrawFacesInRange(int, int);
    virtual int NumFaces() const { return mNumFaces; }
    virtual int NumVerts() const { return mNumVerts; }

    D3DVertexBuffer *GetMultimeshFaces();
    bool OwnerCanDraw() const {
        DxMesh *owner = static_cast<DxMesh *>(mGeomOwner.Ptr());
        return owner->CanDraw();
    }

    NEW_OBJ(DxMesh)

    POOL_OVERLOAD(DxMesh, 0x56);

protected:
    DxMesh();

    virtual void OnSync(int);

    static D3DVertexDeclaration *sVertexDecl;
    static D3DVertexDeclaration *sMutableVertexDecl;
    static D3DVertexDeclaration *sMutableSkinnedVertexDecl;

    unsigned int VertSize() const;
    unsigned int VertFVF() const;
    bool CanDraw() const;
    void Fill(Vert *, Vert *);
    void FillCompressedVerts();
    void SetTransforms();
    DxMat *DrawFur(DxMat *);
    bool CheckFurTransformCache();
    float FurWeight(RndMat *);
    void CacheFurTransform(const Transform &, int, float);

    std::vector<Transform> mTransformCache; // 0x190
    int mNumVerts; // 0x19c
    int mNumFaces; // 0x1a0
    VertexBufferData mVBVerts; // 0x1a4
    D3DIndexBuffer *mIBFaces; // 0x1ac
    D3DVertexBuffer *mIBMultimeshFaces; // 0x1b0
};
