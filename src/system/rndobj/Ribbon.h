#pragma once
#include "math/Key.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include <vector>
#include "types.h"

/** "Ribbon" */
class RndRibbon : public RndPollable, public RndDrawable {
public:
    // Hmx::Object
    virtual ~RndRibbon();
    OBJ_CLASSNAME(Ribbon);
    OBJ_SET_TYPE(Ribbon);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(RndRibbon)
    REGISTER_OBJ(RndRibbon)

    void ConstructMesh();
    void UpdateChase();

protected:
    RndRibbon();
    void ExposeMesh(void);
    void SetActive(bool);

    float unk48; // 0x48
    /** "Number of sides, the more sides, the more cylindrical it is".
        Ranges from 3 to 20. */
    int mNumSides; // 0x4c
    RndMesh *mMesh; // 0x50
    /** "Material to use" */
    ObjPtr<RndMat> mMat; // 0x54
    /** "Width of the tube" */
    float mWidth; // 0x68
    int mDirty; // 0x6c
    bool mActive; // 0x70
    Keys<Transform, Transform> mTransforms; // 0x74
    /** "Number of segments it's built out of". Ranges from 0 to 1000. */
    int mNumSegments; // 0x80
    /** "The ribbon ends after this many seconds.
        Each segment gets an equal fraction of this time".
        Ranges from 1e-3 to 10. */
    float mDecay; // 0x84
    /** "Which Trans to follow" */
    ObjPtr<RndTransformable> mFollowA; // 0x88
    /** "Another Trans to follow" */
    ObjPtr<RndTransformable> mFollowB; // 0x9c
    /** "Interpolates the follow point between [follow_a] and [follow_b],
        so for example .5 gives you the average of those two positions".
        Ranges from 0 to 1. */
    float mFollowWeight; // 0xb0
    /** "Tapers the end if true, otherwise is squared off" */
    bool mTaper; // 0xb4
};
