#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Mesh.h"
#include "utl/PoolAlloc.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"

class DxMesh : public RndMesh, public DxObject {
public:
    struct VertexBufferData {
        VertexBufferData() : mBuffer(0), mSize(0) {}
        ~VertexBufferData() { Release(); }
        void Release();
        void SetData(D3DVertexBuffer *, unsigned int);

        D3DVertexBuffer *mBuffer;
        unsigned int mSize;
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
    virtual void OnSync(int);

    D3DVertexBuffer *GetMultimeshFaces();

    NEW_OBJ(DxMesh)

    POOL_OVERLOAD(DxMesh, 0x56);

protected:
    DxMesh();

    static D3DVertexDeclaration *sVertexDecl;
    static D3DVertexDeclaration *sMutableVertexDecl;
    static D3DVertexDeclaration *sMutableSkinnedVertexDecl;

    unsigned int VertSize() const;
    unsigned int VertFVF() const;
    bool CanDraw() const;
    void Fill(Vert *, Vert *);
    void FillCompressedVerts();

    std::vector<Transform> mTransformCache; // 0x190
    int mNumVerts; // 0x19c
    int mNumFaces; // 0x1a0
    VertexBufferData mVertexBufferData; // 0x1a4
    D3DIndexBuffer *unk1ac; // 0x1ac
    D3DResource *unk1b0; // 0x1b0
};
