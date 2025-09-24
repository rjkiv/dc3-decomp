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
        kFingerThumb,
        kFingerIndex,
        kFingerMiddle,
        kFingerRing,
        kFingerPinky,
        kFingerNone
    };
    struct FingerDesc {
        FingerDesc()
            : unk0(0), unk8(0, 0, 0), unk18(0, 0, 0), mFinger01(nullptr),
              mFinger02(nullptr), mFinger03(nullptr), mFingertip(nullptr), unk88(0),
              unk8c(0), unk90(1) {}
        bool unk0;
        float unk4;
        Vector3 unk8;
        Vector3 unk18;
        ObjPtr<RndTransformable> mFinger01; // 0x28
        ObjPtr<RndTransformable> mFinger02; // 0x3c
        ObjPtr<RndTransformable> mFinger03; // 0x50
        ObjPtr<RndTransformable> mFingertip; // 0x64
        float unk78;
        float unk7c;
        float unk80;
        float unk84;
        int unk88;
        int unk8c;
        bool unk90;
        Vector3 unk94;
        Vector3 unka4;
        bool unkb4;
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

    ObjPtr<RndTransformable> mHand; // 0x30
    ObjPtr<RndTransformable> mForeArm; // 0x44
    ObjPtr<RndTransformable> mUpperArm; // 0x58
    int mBlendInFrames; // 0x6c
    int mBlendOutFrames; // 0x70
    bool mResetHandDest; // 0x74
    bool mResetCurHandTrans; // 0x75
    Transform mCurHandTrans; // 0x78
    Transform mDestHandTrans; // 0xc8
    float mFingerCurledLength; // 0xf8
    Vector3 mDestForwardVector; // 0xfc
    Vector3 mCurForwardVector; // 0x10c
    /** "Starting hand offset from keyboard." */
    Vector3 mHandKeyboardOffset; // 0x11c
    Hmx::Matrix3 mtx; // 0x12c
    /** "how much to move forward when pinky or thumb is engaged" */
    float mHandMoveForward; // 0x15c
    /** "how much to rotate the hand (radians) when pinky is engaged" */
    float mHandPinkyRotation; // 0x160
    /** "how much to rotate the hand (radians) when thumb is engaged" */
    float mHandThumbRotation; // 0x164
    /** "x offset for right/left hands from average destination position for fingers" */
    float mHandDestOffset; // 0x168
    /** "Does this run the right or left hand?" */
    bool mIsRightHand; // 0x16c
    bool unk16d;
    bool mIsSetup; // 0x16e
    std::vector<FingerDesc> mFingers; // 0x170
    int unk17c;
    int unk180;
    /** "This trans will be set to the desired hand position." */
    ObjPtr<RndTransformable> mOutputTrans; // 0x184
    /** "A keyboard bone so we can calculate in local space. use rh/lh targets." */
    ObjPtr<RndTransformable> mKeyboardRefBone; // 0x198
};
