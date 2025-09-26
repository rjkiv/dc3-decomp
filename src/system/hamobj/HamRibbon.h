#pragma once
#include "math/Key.h"
#include "math/Mtx.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Ribbon" */
class HamRibbon : public RndPollable, public RndDrawable {
public:
    // Hmx::Object
    virtual ~HamRibbon();
    OBJ_CLASSNAME(HamRibbon);
    OBJ_SET_TYPE(HamRibbon);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(HamRibbon)

    void Reset();
    void ConstructMesh();
    void UpdateChase();

protected:
    HamRibbon();

    void SetActive(bool);
    void ExposeMesh();

    bool unk48; // 0x48
    float unk4c; // 0x4c
    int mNumSides; // 0x50
    RndMesh *mMesh; // 0x54
    ObjPtr<RndMat> mMat; // 0x58
    float mWidth; // 0x6c
    int unk70; // 0x70
    bool mActive; // 0x74
    ObjPtrList<RndTransformable> unk78; // 0x78
    Keys<Transform, Transform> unk8c; // 0x8c
    int mNumSegments; // 0x98
    float mDecay; // 0x9c
    ObjPtr<RndTransformable> mFollowA; // 0xa0
    ObjPtr<RndTransformable> mFollowB; // 0xb4
    float mFollowWeight; // 0xc8
    bool mTaper; // 0xcc
};
