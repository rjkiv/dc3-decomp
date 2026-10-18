#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "math/Mtx.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Pins fingers to world positions" */
class CharIKFingers : public RndHighlightable,
                      public CharWeightable,
                      public CharPollable {
public:
    enum FingerNum {
        kFingerThumb = 0x0000,
        kFingerIndex = 0x0001,
        kFingerMiddle = 0x0002,
        kFingerRing = 0x0003,
        kFingerPinky = 0x0004,
        kFingerNone = 0x0005
    };
    struct FingerDesc {
        FingerDesc()
            : mIsControlled(0), mDestPos(0, 0, 0), mSecondDestPos(0, 0, 0),
              mBone1(nullptr), mBone2(nullptr), mBone3(nullptr), mBoneTip(nullptr),
              mFBlendInFrames(0), mFBlendOutFrames(0), mResetFingerRots(1) {}

        bool mIsControlled; // 0x0
        float mLength; // 0x4
        Vector3 mDestPos; // 0x8
        Vector3 mSecondDestPos; // 0x18
        ObjPtr<RndTransformable> mBone1; // 0x28
        ObjPtr<RndTransformable> mBone2; // 0x3c
        ObjPtr<RndTransformable> mBone3; // 0x50
        ObjPtr<RndTransformable> mBoneTip; // 0x64
        float mBone2DestAngle; // 0x78
        float mBone3DestAngle; // 0x7c
        float mBone2CurAngle; // 0x80
        float mBone3CurAngle; // 0x84
        int mFBlendInFrames; // 0x88
        int mFBlendOutFrames; // 0x8c
        bool mResetFingerRots; // 0x90
        Vector3 mDestForwardVector; // 0x94
        Vector3 mCurForwardVector; // 0xa4
        bool mDestChanged; // 0xb4
    };
    // Hmx::Object
    virtual ~CharIKFingers();
    OBJ_CLASSNAME(CharIKFingers);
    OBJ_SET_TYPE(CharIKFingers);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetName(const char *, class ObjectDir *);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1F)
    NEW_OBJ(CharIKFingers)

    void MeasureLengths();

protected:
    CharIKFingers();
    void CalculateHandDest(int, int);
    void CalculateFingerDest(FingerNum);
    void MoveFinger(FingerNum);
    void FixSingleFinger(RndTransformable *, RndTransformable *, RndTransformable *);

    ObjPtr<RndTransformable> mHand; // 0x30
    ObjPtr<RndTransformable> mElbow; // 0x44
    ObjPtr<RndTransformable> mShoulder; // 0x58
    int mBlendInFrames; // 0x6c
    int mBlendOutFrames; // 0x70
    bool mResetHandDest; // 0x74
    bool mResetCurHandTrans; // 0x75
    Transform mCurHandTrans; // 0x78
    Transform mDestHandTrans; // 0xb8
    float mFingerCurledLength; // 0xf8
    Vector3 mDestForwardVector; // 0xfc
    Vector3 mCurForwardVector; // 0x10c
    /** "Starting hand offset from keyboard." */
    Vector3 mDefaultOffset; // 0x11c
    Hmx::Matrix3 mDefaultRotation; // 0x12c
    /** "how much to move forward when pinky or thumb is engaged" */
    float mMoveForward; // 0x15c
    /** "how much to rotate the hand (radians) when pinky is engaged" */
    float mPinkyRot; // 0x160
    /** "how much to rotate the hand (radians) when thumb is engaged" */
    float mThumbRot; // 0x164
    /** "x offset for right/left hands from average destination position for fingers" */
    float mDestPosOffset; // 0x168
    /** "Does this run the right or left hand?" */
    bool mIsRightHand; // 0x16c
    bool mMoveHand; // 0x16d
    bool mIsSetup; // 0x16e
    std::vector<FingerDesc> mFingers; // 0x170
    float mInv2ab; // 0x17c
    float mAAPlusBB; // 0x180
    /** "This trans will be set to the desired hand position." */
    ObjPtr<RndTransformable> mHandOutputTrans; // 0x184
    /** "A keyboard bone so we can calculate in local space. use rh/lh targets." */
    ObjPtr<RndTransformable> mKeyboardRefBone; // 0x198
};
