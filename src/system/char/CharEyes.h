#pragma once
#include "CharEyeDartRuleset.h"
#include "CharInterest.h"
#include "char/CharFaceServo.h"
#include "char/CharInterest.h"
#include "char/CharLookAt.h"
#include "char/CharPollable.h"
#include "char/CharWeightSetter.h"
#include "char/CharWeightable.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Overlay.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Moves a bunch of lookats around" */
class CharEyes : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    struct EyeDesc {
        EyeDesc(Hmx::Object *owner)
            : mEye(owner), mUpperLid(owner), mLowerLid(owner), mLowerLidBlink(owner),
              mUpperLidBlink(owner) {}
        EyeDesc &operator=(const EyeDesc &desc) {
            mEye = desc.mEye.Ptr();
            mUpperLid = desc.mUpperLid;
            mLowerLid = desc.mLowerLid;
            mUpperLidBlink = desc.mUpperLidBlink;
            mLowerLidBlink = desc.mLowerLidBlink;
            return *this;
        }

        /** "Eye to retarget" */
        ObjOwnerPtr<CharLookAt> mEye; // 0x0
        /** "corresponding upper lid bone, must rotate about Z" */
        ObjPtr<RndTransformable> mUpperLid; // 0x14
        /** "corresponding lower lid bone, must rotate about Z" */
        ObjPtr<RndTransformable> mLowerLid; // 0x28
        /** "optional - child of lower_lid, placed at edge of lower lid geometry.
            It will be used to detect and resolve interpenetration of the lids" */
        ObjPtr<RndTransformable> mLowerLidBlink; // 0x3c
        /** "optional - child of upper_lid, placed at edge of upper lid geometry.
            It will be used to detect and resolve interpenetration of the lids" */
        ObjPtr<RndTransformable> mUpperLidBlink; // 0x50
    };
    struct CharInterestState {
        CharInterestState(Hmx::Object *owner) : mInterest(owner), unk14(-1) {}
        CharInterestState(const CharInterestState &s)
            : mInterest(s.mInterest), unk14(-1) {}
        CharInterestState &operator=(const CharInterestState &s) {
            mInterest = s.mInterest.Ptr();
            return *this;
        }

        bool IsInRefractoryPeriod();
        float RefractoryTimeRemaining();

        ObjOwnerPtr<CharInterest> mInterest; // 0x0
        float unk14; // 0x14
    };

    // Hmx::Object
    virtual ~CharEyes();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CharEyes);
    OBJ_SET_TYPE(CharEyes);
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
    virtual void Exit();
    virtual void ListPollChildren(std::list<RndPollable *> &) const;
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x20)
    NEW_OBJ(CharEyes)

    void SetInterestFilterFlags(int i) { mInterestFilterFlags = i; }
    void ClearInterestFilterFlags() { mInterestFilterFlags = mDefaultFilterFlags; }
    void SetUnk1b0(bool b) { mInterestFiltersChanged = b; } // change once context found

    void ForceBlink();
    CharInterest *GetCurrentInterest();
    void SetEnableBlinks(bool, bool);
    bool SetFocusInterest(CharInterest *, int);
    void ToggleInterestsDebugOverlay();
    void ClearAllInterestObjects();
    void AddInterestObject(CharInterest *);
    int NumInterests() const { return mInterests.size(); }
    CharInterest *GetInterest(int idx) {
        return idx >= mInterests.size() ? 0 : mInterests[idx].mInterest.Ptr();
    }

protected:
    CharEyes();
    bool IsHeadIKWeightIncreasing();
    void ProceduralBlinkUpdate();
    RndTransformable *GetHead();
    RndTransformable *GetTarget();
    void EnforceMinimumTargetDistance(const Vector3 &, const Vector3 &, Vector3 &);
    void DartUpdate();
    bool EyesOnTarget(float);
    Vector3 GenerateDartOffset();
    void UpdateOverlay();
    void NextLook();
    bool EitherEyeClamped();
    void LidTrackAndClampingUpdate(EyeDesc &, float);

    DataNode OnAddInterest(DataArray *);
    DataNode OnToggleForceFocus(DataArray *);
    DataNode OnToggleInterestOverlay(DataArray *);

    /** "globally disables eye darts for all characters" */
    static bool sDisableEyeDart;
    /** "globally disables eye jitter for all characters" */
    static bool sDisableEyeJitter;
    /** "globally disables use of interest objects for all characters" */
    static bool sDisableInterestObjects;
    /** "globally disables use of procedural blinks for all characters" */
    static bool sDisableProceduralBlink;
    /** "globally disables eye lid clamping on all characters" */
    static bool sDisableEyeClamping;

    ObjVector<EyeDesc> mEyes; // 0x30
    ObjVector<CharInterestState> mInterests; // 0x40
    /** "the CharFaceServo if any, used to allow blinks through the eyelid following" */
    ObjPtr<CharFaceServo> mFaceServo; // 0x50
    /** "The weight setter for eyes tracking the camera" */
    ObjPtr<CharWeightSetter> mCamWeight; // 0x64
    Vector3 unk78; // 0x78
    int mDefaultFilterFlags; // 0x88
    /** "optional bone that serves as the reference for which direction
        the character is looking. If not set, one of the eyes will be used" */
    ObjPtr<RndTransformable> mViewDirection; // 0x8c
    /** "optionally supply a head lookat to inform eyes what the head is doing.
        used primarily to coordinate eye lookats with head ones..." */
    ObjPtr<CharLookAt> mHeadLookAt; // 0xa0
    /** "in degrees, the maximum angle we can offset the current view direction
        when extrapolating for generated interests". Ranges from 0 to 90. */
    float mMaxExtrapolation; // 0xb4
    /** "the minimum distance, in inches, that this interest can be from the eyes.
        If the interest is less than this distance, the eyes look in the same direction,
        but projected out to this distance.  May be overridden per interest object." */
    float mMinTargetDist; // 0xb8
    /** "affects rotation applied to upper lid when eyes rotate up".
        Ranges from 0 to 10. */
    float mUpperLidTrackUp; // 0xbc
    /** "affects rotation applied to upper lid when eyes rotate down".
        Ranges from 0 to 10. */
    float mUpperLidTrackDown; // 0xc0
    /** "translates lower lids up/down when eyes rotate up".
        Ranges from 0 to 10. */
    float mLowerLidTrackUp; // 0xc4
    /** "translates lower lids up/down when eyes rotate down".
        Ranges from 0 to 10. */
    float mLowerLidTrackDown; // 0xc8
    /** "if checked, lower lid tracking is done by rotation instead of translation" */
    bool mLowerLidTrackRotate; // 0xcc
    RndOverlay *mEyeStatusOverlay; // 0xd0
    int mInterestFilterFlags; // 0xd4
    Vector3 mLastFacing; // 0xd8
    float mLastCang;
    float mLastLook;
    float mMaxEyeCang;
    float mAvDelta;
    float mLastBlinkWeight;
    bool mBlinkDetect;
    bool mTargetTooClose;
    ObjPtr<CharInterest> mCurInterest; // 0x100
    ObjPtr<CharInterest> mFocusInterest; // 0x114 - focus interest
    int mCurFocusPriorityClass;
    bool mNewFocusInterest;
    Vector3 mLastExtrapolatedDir;
    float mLastHeadIKWeight;
    CharEyeDartRuleset::EyeDartRulesetData mCurDartRuleset; // 0x144
    bool mDarting;
    float mDartNextEventTime;
    int mDartsRemaining;
    Vector3 mCurDartOffset;
    bool mProceduralBlink;
    float mBlinkTimestamp;
    int mBlinkWindowCount;
    float mBlinkWindowSecsRemaining;
    float mLastBlinkDetectTime;
    Vector3 mDelayedTarget;
    bool mInterestFiltersChanged;
    bool mBlinksEnabled;
};
