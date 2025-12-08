#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Puts a head bone into a position, doing IK on the spine to achieve it." */
class CharIKHead : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    struct Point {
        Point(Hmx::Object *);
        Point(CharIKHead::Point const &);

        ObjPtr<RndTransformable> unk0;
        Vector3 unk14;
        float unk18;
        float unk1c;
        Vector3 unk20;
    };
    // Hmx::Object
    virtual ~CharIKHead();
    OBJ_CLASSNAME(CharIKHead);
    OBJ_SET_TYPE(CharIKHead);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(CharIKHead)

protected:
    CharIKHead();

    void UpdatePoints(bool);

    ObjVector<Point> mPoints; // 0x30
    /** "The head bone which will be moved to the target" */
    ObjPtr<RndTransformable> mHead; // 0x40
    /** "The first spine bone in the chain to be moved" */
    ObjPtr<RndTransformable> mSpine; // 0x54
    /** "If non null, will be the thing that actually hits the target" */
    ObjPtr<RndTransformable> mMouth; // 0x68
    /** "The target to hit" */
    ObjPtr<RndTransformable> mTarget; // 0x7c
    Vector3 mHeadFilter; // 0x90
    /** "allowable head movement relative to the target" */
    float mTargetRadius; // 0xa0
    /** "how much to blend the original world space head matrix in" */
    float mHeadMat; // 0xa4
    /** "Another bone to get the same world space delta" */
    ObjPtr<RndTransformable> mOffset; // 0xa8
    /** "World space axis scaling to apply to delta before applying to offset bone." */
    Vector3 mOffsetScale; // 0xbc
    float mSpineLength; // 0xcc
    bool mUpdatePoints; // 0xd0
    Vector3 mDebugTarget; // 0xd4
};
