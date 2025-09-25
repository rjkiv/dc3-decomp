#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "math/Mtx.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Moves an RndTransformable (bone) to another RndTransformable (spot) over time,
    blending from where it was relative to the parent of the spot." */
class CharIKMidi : public RndHighlightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharIKMidi();
    OBJ_CLASSNAME(CharIKMidi);
    OBJ_SET_TYPE(CharIKMidi);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(CharIKMidi)

    void NewSpot(RndTransformable *, float);

protected:
    CharIKMidi();

    /** "The bone to move" */
    ObjPtr<RndTransformable> mBone; // 0x10
    /** "Spot to go to, zero indexed" */
    ObjPtr<RndTransformable> mCurSpot; // 0x24
    ObjPtr<RndTransformable> mNewSpot; // 0x38
    Transform mLocalXfm; // 0x4c
    Transform mOldLocalXfm; // 0x8c
    float mFrac; // 0xcc
    float mFracPerBeat; // 0xd0
    bool mSpotChanged; // 0xd4
    /** "Weightable to change animation between frets" */
    ObjPtr<CharWeightable> mAnimBlender; // 0xd8
    /** "Max weight for animation change" */
    float mMaxAnimBlend; // 0xec
    float mAnimFracPerBeat; // 0xf0
    float mAnimFrac; // 0xf4
};
