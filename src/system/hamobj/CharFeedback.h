#pragma once
#include "char/Character.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

enum FeedbackLimbs {
    kFeedbackNone = 0,
    kFeedbackLeftArm = 1,
    kFeedbackRightArm = 2,
    kFeedbackLeftLeg = 4,
    kFeedbackRightLeg = 8,
    kNumLimbFeedbacks = 4
};

/** "Drawable for on-character filter feedback/visualization" */
class CharFeedback : public RndDrawable, public RndPollable {
public:
    // size 0x20
    struct LimbState {
        LimbState() : unkc(nullptr) {}
        bool unk0; // 0x0
        bool unk1; // 0x1
        float unk4; // 0x4
        float unk8; // 0x8
        ObjPtr<RndMesh> unkc; // 0xc
    };
    // Hmx::Object
    virtual ~CharFeedback();
    OBJ_CLASSNAME(CharFeedback);
    OBJ_SET_TYPE(CharFeedback);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x12)
    NEW_OBJ(CharFeedback)

    void UpdateLimb(int, bool);
    void ResetErrors();

    static bool sEnabled;

private:
    void Sync();
    void TestUpdateLimbs(bool);

protected:
    CharFeedback();

    /** "Character for displaying filter feedback" */
    ObjPtr<Character> mTarget; // 0x48
    /** "Mat to show on the character" */
    ObjPtr<RndMat> mFailMat; // 0x5c
    /** "Min time a node must be failing to start flash" */
    float mFailTriggerSecs; // 0x70
    /** "Min time a node must be flashing to stop" */
    float mMinFailSecs; // 0x74
    /** "Time (secs) to fade in/out" */
    float mFadeSecs; // 0x78
    LimbState mLimbStates[4]; // 0x7c
    /** "Test limb(s) for debugging" */
    FeedbackLimbs mTestLimbs; // 0xfc
};
