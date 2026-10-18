#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include <vector>

/** "Apply a signal to char bones" */
class CharSignalApplier : public CharPollable, public CharWeightable {
public:
    enum SignalOperation {
        kApplyRotationX = 0x0000,
        kApplyRotationY = 0x0001,
        kApplyRotationZ = 0x0002
    };

    /** "Operation to perform based on the input signal" */
    struct BoneOp {
        BoneOp(Hmx::Object *owner)
            : mBone(owner), mOp(0), mApplyPct(1), mMinAngle(-30), mMaxAngle(30) {}

        /** "Bone to affect" */
        ObjPtr<RndTransformable> mBone; // 0x0
        /** "Operation to perform".
            Options are: "kApplyRotationX" "kApplyRotationY" "kApplyRotationZ" */
        int mOp; // 0x14
        /** "Percentage of effect" */
        float mApplyPct; // 0x18
        /** "minimum signal->rotation of this angle" */
        float mMinAngle; // 0x1c
        /** "maximum signal->rotation of this angle" */
        float mMaxAngle; // 0x20
    };

    // Hmx::Object
    OBJ_CLASSNAME(CharSignalApplier);
    OBJ_SET_TYPE(CharSignalApplier);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(CharSignalApplier);

protected:
    CharSignalApplier();

    /** "Signal value" */
    float mSignalValue; // 0x28
    /** "Signal minimum value" */
    float mSignalMin; // 0x2c
    /** "Signal maximum value" */
    float mSignalMax; // 0x30
    /** "Smooth signal on a per-poll basis" */
    bool mUseSmoothing; // 0x34
    /** "How much can the signal change per poll?" */
    float mPollSignalStep; // 0x38
    float mSmoothedSignal; // 0x3c
    ObjVector<BoneOp> mBoneOps; // 0x40
};
