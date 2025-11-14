#pragma once
#include "char/CharBonesMeshes.h"
#include "char/CharPollable.h"
#include "char/Waypoint.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "utl/Symbol.h"

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
    void Regulate();
    void SetRegulateWaypoint(Waypoint *wp) { mRegulate = wp; }

    NEW_OBJ(CharServoBone)

    int unk84;
    float *mFacingRotDelta; // 0x88
    Vector3 *mFacingPosDelta; // 0x8c
    float *mFacingRot; // 0x90
    Vector3 *mFacingPos; // 0x94
    bool mMoveSelf; // 0x98
    bool mDeltaChanged; // 0x99
    Symbol mClipType; // 0x9c
    ObjPtr<Waypoint> mRegulate; // 0xa0

protected:
    // CharBonesMeshes
    virtual void ReallocateInternal();

    CharServoBone();
};
