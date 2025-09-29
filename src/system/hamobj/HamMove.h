#pragma once
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/ScoreUtl.h"
#include "obj/Object.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

class MoveFrame {
public:
    float mBeat; // 0x0
    int unk4; // 0x4
};

/** "Data associated with a ham Move" */
class HamMove : public RndPropAnim {
public:
    struct LocalizedName {
        Symbol mLanguage; // 0x0
        String mName; // 0x4
    };
    enum TexState {
        kTexNone = 0,
        kTexNormal = 1,
        kTexFlip = 2,
        kTexDblFlip = 3
    };
    // Hmx::Object
    virtual ~HamMove();
    OBJ_CLASSNAME(HamMove);
    OBJ_SET_TYPE(HamMove);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPropAnim
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x68)
    NEW_OBJ(HamMove)
    static float sMinFrameDistBeats;

    void SetTexture(RndTex *);
    bool IsRest() const;
    bool IsFinalPose() const;
    bool SuppressGuideGesture() const;
    bool SuppressPracticeOptions() const;
    void RefreshBarks();
    float Confusability(const HamMove *) const;
    const char *DisplayName() const;
    float AdjustNormalizedPercentToConfusability(float, float);
    float ConfusabilityWithMoveDataArray(const DataArray *);

protected:
    HamMove();

    void SyncMirror();

    /** "Move to mirror" */
    ObjPtr<HamMove> mMirror; // 0x30
    /** "Texture to describe the move" */
    ObjPtr<RndTex> mTex; // 0x44
    ObjPtr<RndTex> mSmallTex; // 0x58
    /** "Texture state describes how to display the tex" */
    TexState mTexState; // 0x6c
    std::vector<MoveFrame> mMoveFrames; // 0x70
    /** "True if this is move is scored.
        False if it's a rest or some kind of indicator (like freestyle)" */
    bool mScored; // 0x7c
    /** "True if this move is a paradiddle" */
    bool mParadiddle; // 0x7d
    /** "True if this move is the final pose in the song" */
    bool mFinalPose; // 0x7e
    /** "Prevent the Guide Gesture from appearing for the duration of this move" */
    bool mSuppressGuide; // 0x7f
    /** "Prevent the Practice Options from appearing for the duration of this move" */
    bool mSuppressPracticeOptions; // 0x80
    /** "Prevent this move from appear in the dance battle minigame" */
    bool mOmitMinigame; // 0x81
    std::vector<LocalizedName> mLocalizedNames; // 0x84
    const char *mDisplayName; // 0x90
    Difficulty mDifficulty; // 0x94
    Symbol mVerb; // 0x98
    Symbol mMoveSound; // 0x9c
    std::vector<float> mRatingStates; // 0xa0
    /** "Whether to use shoudler displacements for detection" - specific to Ham1! */
    bool mShoulderDisplacements; // 0xac
    /** "Generated threshold for super perfect"/
        "perfect/flawless"/"awesome/nice"/"ok/almost" */
    float mThresholds[kNumMoveRatings]; // 0xb0
    /** "Override threshold for super perfect /
        perfect/flawless / awesome/nice / ok/almost (0 means no override)" */
    float mOverrides[kNumMoveRatings]; // 0xc0
    bool unkd0; // 0xd0
    /** "id used when comparing to other moves" */
    Hmx::CRC mConfusabilityID; // 0xd4
    std::map<Hmx::CRC, float> mConfusabilities; // 0xd8
    ObjPtr<DancerSequence> mDancerSeq; // 0xf0
};
