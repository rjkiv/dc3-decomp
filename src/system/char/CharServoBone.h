#pragma once
#include "char/CharBonesMeshes.h"
#include "char/CharPollable.h"
#include "char/Waypoint.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

/** "Sets bone transforms and regulates Character center to a spot." */
class CharServoBone : public RndHighlightable,
                      public CharPollable,
                      public CharBonesMeshes {
public:
    // Hmx::Object
    virtual ~CharServoBone();
    OBJ_CLASSNAME(CharServoBone);
    OBJ_SET_TYPE(CharServoBone);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight() {}
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    void SetClipType(Symbol);
    void SetMoveSelf(bool);
    void MoveToFacing(Transform &);
    void MoveToDeltaFacing(Transform &);
    void ZeroDeltas();
    void Regulate(Waypoint *wp) { mWay = wp; }
    bool MovingSelf() const { return mMoveSelf; }

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(CharServoBone)

protected:
    // CharBonesMeshes
    virtual void ReallocateInternal();

    void RegulateInternal(class Character *);
    void
    DoRegulate(class Character *, class Waypoint *, class CharClipDriver *, float, float);

    CharServoBone();

    RndTransformable *mPelvis; // 0x84
    float *mFacingRotDelta; // 0x88
    Vector3 *mFacingPosDelta; // 0x8c
    float *mFacingRot; // 0x90
    Vector3 *mFacingPos; // 0x94
    /** "Move ourselves around when playing animations" */
    bool mMoveSelf; // 0x98
    bool mDeltaChanged; // 0x99
    /** "What degrees of freedom we can accomodate" */
    Symbol mClipType; // 0x9c
    /** "Waypoint to regulate to" */
    ObjPtr<Waypoint> mWay; // 0xa0
};
