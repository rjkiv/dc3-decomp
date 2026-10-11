#pragma once
#include "char/Character.h"
#include "hamobj/ErrorNode.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

/** "Drawable for on-character filter feedback/visualization" */
class CharFeedback : public RndDrawable, public RndPollable {
public:
    // size 0x20
    struct LimbState {
        LimbState() : mMesh(nullptr) {}

        /**
         * Indicates the current "state" - that being whether the limb is actually
         * "outlined".
         */
        bool mFailing; // 0x0

        /**
         * Indicates whether the limb is currently in a "failing" state. This is used to
         * determine when to start the "outline" effect.
         */
        bool mCurrentlyFailing; // 0x1

        /**
         * The time (in seconds) when the limb last changed state. This is used to
         * determine how long the limb has been in its current state.
         */
        float mStateSeconds; // 0x4

        /**
         * The current alpha value for the material effect. This is used to fade the
         * effect in and out over time.
         */
        float mMatAlpha; // 0x8

        /**
         * The mesh associated with this limb. This is used to apply the material effect
         * to the correct part of the character.
         */
        ObjPtr<RndMesh> mMesh; // 0xc
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
    // RndDrawable
    virtual void DrawShowing();

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
