#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"

/** "Moves an RndTransformable (bone) to a percentage of the way between two spots." */
class CharIKSliderMidi : public RndHighlightable,
                         public CharWeightable,
                         public CharPollable {
public:
    // Hmx::Object
    virtual ~CharIKSliderMidi();
    OBJ_CLASSNAME(CharIKSliderMidi);
    OBJ_SET_TYPE(CharIKSliderMidi);
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

    OBJ_MEM_OVERLOAD(0x1E)
    NEW_OBJ(CharIKSliderMidi)

    void SetFraction(float, float);
    void SetupTransforms();

protected:
    CharIKSliderMidi();

    /** "The bone to move" */
    ObjPtr<RndTransformable> mTarget; // 0x30
    /** "Spot at 0%" */
    ObjPtr<RndTransformable> mFirstSpot; // 0x44
    /** "Spot at 100%" */
    ObjPtr<RndTransformable> mSecondSpot; // 0x58
    Vector3 unk6c;
    Vector3 unk7c;
    Vector3 unk8c;
    float unk9c;
    int unka0;
    float unka4;
    float unka8;
    bool unkac;
    bool mResetAll; // 0xad
    /** "Only move if percentage changes more than this (0.0 - 1.0)" */
    float mTolerance; // 0xb0
};
