#pragma once
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/ScoreUtl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

enum MoveMirrored {
    kMoveNotMirrored = 0,
    kMoveMirrored = 1,
    kNumMoveMirrored = 2
};

class MoveFrame {
public:
    float Beat() const { return mBeat; }
    int NodeTypeFlags() const { return mNodeTypeFlags; }
    const Ham1NodeWeight &
    NodeWeightHam1(int node, MoveMode mode, MoveMirrored mirror) const;
    const Vector3 &NodeWeight(int node, MoveMirrored mirror) const;
    const Vector3 &NodeInverseScale(int node, MoveMirrored mirror) const;
    const Ham2FrameWeight &FrameWeight(MoveMirrored mirror) const;
    float QuantizedSeconds(float move_start_beat) const;
    // void Init(float);
    // void SetNodeTypeFlags(int);
    // void SetNodeWeight(int, MoveMirrored, const Vector3 &);
    void SetNodeScale(int mode, MoveMirrored mirror, const Vector3 &scale);
    // void SetFrameWeight(MoveMirrored, const Ham2FrameWeight&);
    void Save(BinStream &d) const;
    void Load(BinStreamRev &d);

    static const int kNumHam1Nodes = 16; // for DC1

private:
    float mBeat; // 0x0
    int mNodeTypeFlags; // 0x4
    Ham1NodeWeight mHam1NodeWeights[kNumMoveModes][kNumMoveMirrored][kNumHam1Nodes]; // 0x8
    Vector3 mNodeWeights[kNumMoveMirrored][kMaxNumErrorNodes]; // 0x508
    Vector3 mNodeScales[kNumMoveMirrored][kMaxNumErrorNodes]; // 0x928
    Vector3 mNodesInverseScale[kNumMoveMirrored][kMaxNumErrorNodes]; // 0xd48
    Ham2FrameWeight mFrameWeights[kNumMoveMirrored]; // 0x1168
};

/** "Data associated with a ham Move" */
class HamMove : public RndPropAnim {
public:
    struct LocalizedName {
        bool operator==(const Symbol s) const { return mLanguage == s; }

        /** The localized name's language. (i.e. eng, esp, fre) */
        Symbol mLanguage; // 0x0
        /** The move's name, in that language. */
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
    REGISTER_OBJ(HamMove)
    static float sMinFrameDistBeats;

    const char *DisplayName() const;
    RndTex *Tex() const { return mTex; }
    RndTex *SmallTex() const { return mSmallTex; }
    bool Scored() const { return mScored; }
    bool IsRest() const;
    bool IsFinalPose() const;
    const std::vector<MoveFrame> &GetMoveFrames() const;
    std::vector<MoveFrame> &GetMoveFrames();
    //  public: HamMove* Mirror();
    MoveMirrored Mirrored() const;
    bool SuppressGuideGesture() const;
    bool SuppressPracticeOptions() const;
    //  public: bool OmitMinigame() const;
    const std::vector<float> *RatingOverride() const;
    //  public: bool ShoulderDisplacements() const;
    float PSNRThreshold(MoveRating r) const;
    float PSNRToDetectFrac(float psnr) const;
    float Confusability(const HamMove *move) const;
    //  public: bool Paradiddle();
    //  public: Difficulty GetDifficulty();
    FilterVersionType Version() const;
    const FilterVersion *FilterVer() const;
    void SetTexture(RndTex *tex);
    //  public: void SetScored(bool);
    //  public: void SetPSNRThresholds(float*);
    //  public: void AddMoveFrame(const MoveFrame&);
    //  public: void ClearConfusability();
    //  public: void SetConfusability(const HamMove*, float);
    void Update(const HamMove *altMove);
    void RefreshBarks();
    float
    AdjustNormalizedPercentToConfusability(float normalized_pct, float confusability);
    float ConfusabilityWithMoveDataArray(const DataArray *moves);
    //  public: bool IsLoopable();
    //  public: TextureState TexState() const;
    // LocalizedName *FindName(Symbol);

protected:
    HamMove();

    float FindConfusabilty(const HamMove *move) const;
    //  protected: float RangedDetectionFrac(int32_t, int32_t) const;
    //  protected: float BeatWithinBar(float) const;
    //  protected: const char* FindNameStr(Symbol);
    void SetName(Symbol lang, const char *str);
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
    /** If true, members set in Update(const HamMove*)
        will not be overwritten from a call to Load(BinStream&). */
    bool mUpdateOverride; // 0xd0
public:
    /** "id used when comparing to other moves" */
    Hmx::CRC mConfusabilityID; // 0xd4
    std::map<Hmx::CRC, float> mConfusabilities; // 0xd8
    /** "Dancer sequence to use for this move" */
    ObjPtr<DancerSequence> mDancerSeq; // 0xf0
};

struct HamMoveKey {
    HamMove *move;
    float beat;
};

struct HamMoveScore {
    HamMove *move; // 0x0
    int ratingIdx; // 0x4
    /**
        Rating fraction, from 0 to 1, where:

        Bad (-) >= 0
        Almost (ALMOST) >= 0.5
        OK (NICE!) >= 0.7
        Flawless (FLAWLESS!!!) >= 0.9
        Perfect (FLAWLESS!!!) = 1
    */
    float detectFrac; // 0x8
    /** Used in rehearse mode */
    bool slowMo; // 0xc
};
