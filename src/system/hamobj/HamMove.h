#pragma once
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/ScoreUtl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

enum MoveMode {
    kNumMoveModes = 2
};

enum MoveMirrored {
    kMirroredNo = 0,
    kMirroredYes = 1,
    kNumMoveMirrored = 2
};

class MoveFrame {
public:
    enum {
        // for DC1
        kNumHam1Nodes = 16
    };
    void Save(BinStream &) const;
    void Load(BinStreamRev &);
    const Ham1NodeWeight &NodeWeightHam1(int, MoveMode, MoveMirrored) const;
    const Ham2FrameWeight &FrameWeight(MoveMirrored) const;
    const Vector3 &NodeWeight(int, MoveMirrored) const;
    const Vector3 &NodeInverseScale(int, MoveMirrored) const;
    void SetNodeScale(int, MoveMirrored, const Vector3 &);

private:
    float mBeat; // 0x0
    int unk4; // 0x4
    Ham1NodeWeight mHam1NodeWeights[kNumMoveModes][kNumMoveMirrored][kNumHam1Nodes]; // 0x8
    Vector3 mNodeWeights[kNumMoveMirrored][kMaxNumErrorNodes]; // 0x508
    Vector3 mNodeScales[kNumMoveMirrored][kMaxNumErrorNodes]; // 0x928
    Vector3 mNodesInverseScale[kNumMoveMirrored][kMaxNumErrorNodes]; // 0xd48
    Ham2FrameWeight mFrameWeights[kNumMoveMirrored]; // 0x1168
};

class FilterVersion;

/** "Data associated with a ham Move" */
class HamMove : public RndPropAnim {
public:
    struct LocalizedName {
        bool operator==(const Symbol &s) const { return mLanguage == s; }

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
    std::vector<MoveFrame> &GetMoveFrames();
    const std::vector<MoveFrame> &GetMoveFrames() const;
    MoveMirrored Mirrored() const;
    void Update(const HamMove *);
    const FilterVersion *FilterVer() const;

    bool Scored() const { return mScored; }
    DancerSequence *GetDancerSequence() const { return mDancerSeq; }

protected:
    HamMove();

    void SyncMirror();
    float FindConfusabilty(const HamMove *) const;
    void SetName(Symbol, const char *);

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

struct HamMoveKey {
    HamMove *move;
    float beat;
};
