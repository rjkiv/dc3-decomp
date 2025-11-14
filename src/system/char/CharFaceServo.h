#pragma once
#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharPollable.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

/** "BonesMeshes for facial blending" */
class CharFaceServo : public CharPollable, public CharBonesMeshes {
public:
    // Hmx::Object
    virtual ~CharFaceServo();
    OBJ_CLASSNAME(CharFaceServo);
    OBJ_SET_TYPE(CharFaceServo);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &list) {
        StuffMeshes(list);
    }
    // CharBonesMeshes
    virtual void ScaleAdd(CharClip *, float, float, float);

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(CharFaceServo)

    void SetClips(ObjectDir *);
    void SetClipType(Symbol);
    void SetBlinkClipLeft(Symbol);
    void SetBlinkClipRight(Symbol);
    float BlinkWeightLeft() const;
    void ApplyProceduralWeights();
    CharClip *BaseClip() const { return mBaseClip; }
    void SetProceduralBlinkWeight(float weight) { unk114 = weight; }

protected:
    CharFaceServo();

    virtual void ReallocateInternal() { CharBonesMeshes::ReallocateInternal(); }

    /** "pointer to visemes, must contain Blink and Base" */
    ObjPtr<ObjectDir> mClips; // 0x7c
    /** "Which clip type it can support" */
    Symbol mClipType; // 0x90
    ObjPtr<CharClip> mBaseClip; // 0x94
    /** "Blink clip, used to close the left eye" */
    Symbol mBlinkClipLeftName; // 0xa8
    /** "A second clip that contributes to closing the left eye" */
    Symbol mBlinkClipLeftName2; // 0xac
    /** "Blink clip, used to close the right eye" */
    Symbol mBlinkClipRightName; // 0xb0
    /** "A second clip that contributes to closing the right eye" */
    Symbol mBlinkClipRightName2; // 0xb4
    ObjPtr<CharClip> mBlinkClipLeft; // 0xb8
    ObjPtr<CharClip> mBlinkClipLeft2; // 0xcc
    ObjPtr<CharClip> mBlinkClipRight; // 0xe0
    ObjPtr<CharClip> mBlinkClipRight2; // 0xf4
    float mBlinkWeightLeft; // 0x108
    float mBlinkWeightRight; // 0x10c
    bool unk110; // 0x110
    float unk114; // 0x114
    bool unk118; // 0x118
};
