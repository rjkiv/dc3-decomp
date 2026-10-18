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
            : eye(owner), upperLid(owner), lowerLid(owner), lowerLidBlink(owner),
              upperLidBlink(owner) {}
        EyeDesc &operator=(const EyeDesc &desc) {
            eye = desc.eye.Ptr();
            upperLid = desc.upperLid;
            lowerLid = desc.lowerLid;
            upperLidBlink = desc.upperLidBlink;
            lowerLidBlink = desc.lowerLidBlink;
            return *this;
        }

        /** "Eye to retarget" */
        ObjOwnerPtr<CharLookAt> eye; // 0x0
        /** "corresponding upper lid bone, must rotate about Z" */
        ObjPtr<RndTransformable> upperLid; // 0x14
        /** "corresponding lower lid bone, must rotate about Z" */
        ObjPtr<RndTransformable> lowerLid; // 0x28
        /** "optional - child of lower_lid, placed at edge of lower lid geometry.
            It will be used to detect and resolve interpenetration of the lids" */
        ObjPtr<RndTransformable> lowerLidBlink; // 0x3c
        /** "optional - child of upper_lid, placed at edge of upper lid geometry.
            It will be used to detect and resolve interpenetration of the lids" */
        ObjPtr<RndTransformable> upperLidBlink; // 0x50
    };

    struct CharInterestState {
        CharInterestState(Hmx::Object *owner) : interest(owner), mLastTimeStamp(-1) {}
        CharInterestState(const CharInterestState &s)
            : interest(s.interest), mLastTimeStamp(-1) {}
        CharInterestState &operator=(const CharInterestState &s) {
            interest = s.interest.Ptr();
            return *this;
        }

        bool IsInRefractoryPeriod();
        float RefractoryTimeRemaining();
        void SetLastTimeStamp(float time) { mLastTimeStamp = time; }

        ObjOwnerPtr<CharInterest> interest; // 0x0

    protected:
        float mLastTimeStamp; // 0x14
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

    void SetInterestFilterFlags(int i) { mCurInterestFilterFlags = i; }
    void ClearInterestFilterFlags() {
        mCurInterestFilterFlags = mDefaultInterestFilterFlags;
    }
    void SetInterestFiltersChanged(bool b) { mInterestFiltersChanged = b; }

    void ForceBlink();
    CharInterest *GetCurrentInterest();
    void SetEnableBlinks(bool, bool);
    bool SetFocusInterest(CharInterest *, int);
    void ToggleInterestsDebugOverlay();
    void ClearAllInterestObjects();
    void AddInterestObject(CharInterest *);
    int NumInterests() const { return mInterests.size(); }
    CharInterest *GetInterest(int idx) {
        return idx >= mInterests.size() ? 0 : mInterests[idx].interest.Ptr();
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
    Vector3 mTarget; // 0x78
    int mDefaultInterestFilterFlags; // 0x88
    /** "optional bone that serves as the reference for which direction
        the character is looking. If not set, one of the eyes will be used" */
    ObjPtr<RndTransformable> mViewDirBone; // 0x8c
    /** "optionally supply a head lookat to inform eyes what the head is doing.
        used primarily to coordinate eye lookats with head ones..." */
    ObjPtr<CharLookAt> mHeadLookAt; // 0xa0
    /** "in degrees, the maximum angle we can offset the current view direction
        when extrapolating for generated interests". Ranges from 0 to 90. */
    float mMaxExtrapolationAngle; // 0xb4
    /** "the minimum distance, in inches, that this interest can be from the eyes.
        If the interest is less than this distance, the eyes look in the same direction,
        but projected out to this distance.  May be overridden per interest object." */
    float mMinTargetDistance; // 0xb8
    /** "affects rotation applied to upper lid when eyes rotate up".
        Ranges from 0 to 10. */
    float mUpperLidTrackUpScale; // 0xbc
    /** "affects rotation applied to upper lid when eyes rotate down".
        Ranges from 0 to 10. */
    float mUpperLidTrackDownScale; // 0xc0
    /** "translates lower lids up/down when eyes rotate up".
        Ranges from 0 to 10. */
    float mLowerLidTrackUpScale; // 0xc4
    /** "translates lower lids up/down when eyes rotate down".
        Ranges from 0 to 10. */
    float mLowerLidTrackDownScale; // 0xc8
    /** "if checked, lower lid tracking is done by rotation instead of translation" */
    bool mLLidTrackRotate; // 0xcc
    RndOverlay *mOverlay; // 0xd0
    int mCurInterestFilterFlags; // 0xd4
    Vector3 mLastFacing; // 0xd8
    float mLastCang; // 0xe8
    float mLastLook; // 0xec
    float mMaxEyeCang; // 0xf0
    float mAvDelta; // 0xf4
    float mLastBlinkWeight; // 0xf8
    bool mBlinkDetect; // 0xfc
    bool mTargetTooClose; // 0xfd
    ObjPtr<CharInterest> mCurInterest; // 0x100
    ObjPtr<CharInterest> mFocusInterest; // 0x114
    int mCurFocusPriorityClass; // 0x128
    bool mNewFocusInterest; // 0x12c
    Vector3 mLastExtrapolatedDir; // 0x130
    float mLastHeadIKWeight; // 0x140
    CharEyeDartRuleset::EyeDartRulesetData mCurDartRuleset; // 0x144
    bool mDarting; // 0x170
    float mDartNextEventTime; // 0x174
    int mDartsRemaining; // 0x178
    Vector3 mCurDartOffset; // 0x17c
    bool mProceduralBlink; // 0x18c
    float mBlinkTimestamp; // 0x190
    int mBlinkWindowCount; // 0x194
    float mBlinkWindowSecsRemaining; // 0x198
    float mLastBlinkDetectTime; // 0x19c
    Vector3 mDelayedTarget; // 0x1a0
    bool mInterestFiltersChanged; // 0x1b0
    bool mBlinksEnabled; // 0x1b1
};
