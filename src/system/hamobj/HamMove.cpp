#include "hamobj/HamMove.h"
#include "FilterVersion.h"
#include "gesture/SkeletonClip.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/MoveDir.h"
#include "hamobj/ScoreUtl.h"
#include "hamobj/Difficulty.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "utl/TimeConversion.h"
#include <cmath>

float HamMove::sMinFrameDistBeats = 0.2;

BinStream &operator<<(BinStream &bs, const Ham1NodeWeight &wt) {
    bs << wt.unk4 << wt.unk8 << wt.unkc << wt.unk10 << wt.unk0;
    return bs;
}

BinStream &operator>>(BinStreamRev &d, Ham1NodeWeight &wt) {
    d >> wt.unk4;
    d >> wt.unk8;
    d >> wt.unkc;
    d >> wt.unk10;
    if (d.rev > 39) {
        d >> wt.unk0;
    } else if (d.rev > 32) {
        float f1;
        d >> f1;
        wt.unk0 = f1 != 0;
    } else if (d.rev > 24) {
        d >> wt.unk0;
    } else
        wt.unk0 = 1;
    return d.stream;
}

BinStream &operator>>(BinStreamRev &d, OldNodeWeight &wt) {
    MILO_ASSERT(d.rev < 40, 0xA6);
    d >> wt.unk4;
    d >> wt.unk8;
    d >> wt.unkc;
    d >> wt.unk10;
    if (d.rev > 32) {
        d >> wt.unk0;
    } else if (d.rev > 24) {
        bool b;
        d >> b;
        if (b) {
            wt.unk0 = 1;
        } else {
            wt.unk0 = 0;
        }
    } else {
        wt.unk0 = 1;
    }
    return d.stream;
}

BinStream &operator<<(BinStream &bs, const Ham2FrameWeight &wt) {
    bs << wt.unk0;
    for (int i = 0; i < 4; i++) {
        bs << wt.unk4[i];
        bs << wt.unk14[i];
    }
    return bs;
}

BinStream &operator>>(BinStreamRev &d, Ham2FrameWeight &wt) {
    if (d.rev > 34) {
        d >> wt.unk0;
    }
    if (d.rev < 33) {
        if (d.rev > 31) {
            float f;
            for (int i = 0; i < 20; i++) {
                d >> f;
            }
        } else if (d.rev > 30) {
            float f;
            d >> f;
        }
    }
    if (d.rev < 37) {
        float x, y;
        for (int i = 0; i < 4; i++) {
            d >> x;
            d >> y;
        }
    }

    if (d.rev > 37) {
        for (int i = 0; i < 4; i++) {
            d >> wt.unk4[i];
            d >> wt.unk14[i];
        }
    } else {
        for (int i = 0; i < 4; i++) {
            wt.unk4[i] = 0;
            wt.unk14[i] = kHugeFloat;
        }
    }
    return d.stream;
}

#pragma region MoveFrame

void MoveFrame::Save(BinStream &bs) const {
    bs << mBeat;
    bs << unk4;
    bs << kNumHam1Nodes;
    for (int i = 0; i < kNumMoveModes; i++) {
        for (int j = 0; j < kNumMoveMirrored; j++) {
            for (int k = 0; k < kNumHam1Nodes; k++) {
                bs << mHam1NodeWeights[i][j][k];
            }
        }
    }
    for (int i = 0; i < kNumMoveMirrored; i++) {
        bs << mFrameWeights[i];
    }
    int numHam2Nodes = FilterVersion::NumHam2Nodes();
    bs << numHam2Nodes;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < numHam2Nodes; j++) {
            bs << mNodeWeights[i][j];
            bs << mNodeScales[i][j];
        }
    }
}

void MoveFrame::Load(BinStreamRev &d) {
    d >> mBeat;
    if (d.rev < 14) {
        MILO_FAIL("Versions less than 14 no longer supported");
    }
    if (d.rev > 0x2B) {
        d >> unk4;
    } else
        unk4 = -1;
    int num_ham2_nodes = FilterVersion::NumHam2Nodes();
    int num_ham1_nodes = kNumHam1Nodes;
    if (d.rev > 0x27) {
        d >> num_ham1_nodes;
        MILO_ASSERT(num_ham1_nodes == kNumHam1Nodes, 0x122);
        for (int i = 0; i < kNumMoveModes; i++) {
            for (int j = 0; j < kNumMoveMirrored; j++) {
                for (int k = 0; k < kNumHam1Nodes; k++) {
                    d >> mHam1NodeWeights[i][j][k];
                }
            }
        }
        for (int i = 0; i < kNumMoveMirrored; i++) {
            d >> mFrameWeights[i];
        }
        int count;
        d >> count;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < count; j++) {
                if (j >= num_ham2_nodes) {
                    if (d.rev < 0x29) {
                        float x;
                        d >> x;
                    } else {
                        Vector3 v;
                        d >> v;
                    }
                    if (d.rev > 0x2D) {
                        Vector3 v;
                        d >> v;
                    }
                } else {
                    if (d.rev < 0x29) {
                        float x;
                        d >> x;
                        mNodeWeights[i][j].Set(x, x, x);
                    } else {
                        d >> mNodeWeights[i][j];
                    }
                    if (d.rev < 0x2E) {
                        mNodeScales[i][j].Set(1, 1, 1);
                    } else {
                        d >> mNodeScales[i][j];
                    }
                    for (int k = 0; k < 3; k++) {
                        float &cur = mNodeScales[i][j][k];
                        float set = kHugeFloat;
                        if (0.0000099999997f <= fabsf(cur)) {
                            set = 1.0f / cur;
                        }
                        mNodeScales[i][j][k] = set;
                    }
                }
            }
        }
        for (; count < num_ham2_nodes; count++) {
            for (int mirror = 0; mirror < kNumMoveMirrored; mirror++) {
                SetNodeScale(count, (MoveMirrored)mirror, Vector3(1, 1, 1));
            }
        }
    } else {
        if (d.rev < 0x13) {
            int max = 5;
            if (d.rev < 0x11) {
                max = 4;
            }
            for (int i = 0; i < max; i++) {
                int v;
                d >> v;
            }
        } else if (d.rev < 0x18) {
            int count;
            d >> count;
            Symbol s;
            int v;
            for (int i = 0; i < count; i++) {
                d >> s;
                d >> v;
            }
        }
        std::vector<OldNodeWeight> oldNodeWeights[8];
        if (d.rev < 0x1E) {
            if (d.rev > 0x17) {
                for (int i = 0; i < 2; i++) {
                    d >> oldNodeWeights[i];
                }
            }
            if (d.rev > 0x1B) {
                for (int i = 2; i < 4; i++) {
                    d >> oldNodeWeights[i];
                }
            }
        } else {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        d >> oldNodeWeights[i + k];
                    }
                }
            }
        }
        // more...
    }
}

const Ham1NodeWeight &
MoveFrame::NodeWeightHam1(int node, MoveMode mode, MoveMirrored mirror) const {
    MILO_ASSERT_RANGE(mode, 0, kNumMoveModes, 0x20);
    MILO_ASSERT_RANGE(mirror, 0, kNumMoveMirrored, 0x21);
    MILO_ASSERT_RANGE(node, 0, kNumHam1Nodes, 0x22);
    return mHam1NodeWeights[mode][mirror][node];
}

const Ham2FrameWeight &MoveFrame::FrameWeight(MoveMirrored mirror) const {
    MILO_ASSERT(mFrameWeights, 0x37);
    MILO_ASSERT_RANGE(mirror, 0, kNumMoveMirrored, 0x38);
    return mFrameWeights[mirror];
}

const Vector3 &MoveFrame::NodeWeight(int node, MoveMirrored mirror) const {
    MILO_ASSERT_RANGE(mirror, 0, kNumMoveMirrored, 0x28);
    MILO_ASSERT_RANGE(node, 0, FilterVersion::NumHam2Nodes(), 0x29);
    return mNodeWeights[mirror][node];
}

const Vector3 &MoveFrame::NodeInverseScale(int node, MoveMirrored mirror) const {
    MILO_ASSERT_RANGE(mirror, 0, kNumMoveMirrored, 0x30);
    MILO_ASSERT_RANGE(node, 0, FilterVersion::NumHam2Nodes(), 0x31);
    return mNodesInverseScale[mirror][node];
}

void MoveFrame::SetNodeScale(int node, MoveMirrored mirror, const Vector3 &v) {
    MILO_ASSERT_RANGE(mirror, 0, kNumMoveMirrored, 0x6D);
    MILO_ASSERT_RANGE(node, 0, FilterVersion::NumHam2Nodes(), 0x6E);
    for (int i = 0; i < 3; i++) {
        float s = v[i];
        MILO_ASSERT(s > 0, 0x72);
        mNodeScales[mirror][node][i] = s;
        mNodesInverseScale[mirror][node][i] = 1.0f / s;
    }
}

float MoveFrame::QuantizedSeconds(float f) const {
    float seconds = floor(BeatToSeconds(mBeat + f) * 30.0f + 0.5f);
    return seconds / 30.0f;
}

#pragma endregion
#pragma region HamMove

HamMove::HamMove()
    : mMirror(this), mTex(this), mSmallTex(this), mTexState(kTexNormal), mScored(true),
      mParadiddle(false), mFinalPose(false), mSuppressGuide(false),
      mSuppressPracticeOptions(false), mOmitMinigame(false), mDisplayName(nullptr),
      mDifficulty(kDifficultyExpert), mShoulderDisplacements(false), unkd0(0),
      mDancerSeq(this) {
    SetRate(k480_fpb);
    Symbol lang = SystemLanguage(); // unused lol
    DataArray *supportedLangs = SupportedLanguages(false);
    mLocalizedNames.resize(supportedLangs->Size());
    for (int i = 0; i < supportedLangs->Size(); i++) {
        mLocalizedNames[i].mLanguage = supportedLangs->Sym(i);
    }
    if (TheLoadMgr.EditMode()) {
        static Symbol verb("verb");
        DataArrayPtr ptr(verb);
        AddKeys(this, ptr, PropKeys::kSymbol);
        static Symbol verb_slow("verb_slow");
        DataArrayPtr ptrSlow(verb_slow);
        AddKeys(this, ptrSlow, PropKeys::kSymbol);
    }
    for (int i = 0; i < kNumMoveRatings; i++) {
        mRatingStates.push_back(0);
        // super_perfect = 100, perfect = 75, awesome = 50, ok = 25
        mThresholds[i] = (4 - i) * 25.0f;
        mOverrides[i] = 0;
    }
}

HamMove::~HamMove() {}

BEGIN_HANDLERS(HamMove)
    HANDLE_EXPR(display_name, DisplayName())
    HANDLE_EXPR(is_rest, IsRest())
    HANDLE_ACTION(refresh_barks, RefreshBarks())
    HANDLE_EXPR(confusability, Confusability(_msg->Obj<HamMove>(2)))
    HANDLE_EXPR(
        adjust_normalized_percent_to_confusability,
        AdjustNormalizedPercentToConfusability(_msg->Float(2), _msg->Float(3))
    )
    HANDLE_EXPR(
        confusability_with_move_data_array, ConfusabilityWithMoveDataArray(_msg->Array(2))
    )
    HANDLE_EXPR(is_loopable, true)
    HANDLE_EXPR(has_filters, !mMoveFrames.empty())
    HANDLE_SUPERCLASS(RndPropAnim)
END_HANDLERS

BEGIN_PROPSYNCS(HamMove)
    SYNC_PROP_MODIFY(mirror, mMirror, SyncMirror())
    SYNC_PROP_SET(tex, mTex.Ptr(), SetTexture(_val.Obj<RndTex>()))
    SYNC_PROP(small_tex, mSmallTex)
    SYNC_PROP_SET(tex_state, mTexState, mTexState = (TexState)_val.Int())
    SYNC_PROP(scored, mScored)
    SYNC_PROP(final_pose, mFinalPose)
    SYNC_PROP(verb, mVerb)
    SYNC_PROP(verb_slow, mVerb)
    SYNC_PROP(move_sound, mMoveSound)
    SYNC_PROP(paradiddle, mParadiddle)
    SYNC_PROP(omit_minigame, mOmitMinigame)
    SYNC_PROP(suppress_guide, mSuppressGuide)
    SYNC_PROP(suppress_practice_options, mSuppressPracticeOptions)
    SYNC_PROP_SET(difficulty, (int &)mDifficulty, mDifficulty = (Difficulty)_val.Int())
    SYNC_PROP(move_perfect, mRatingStates[RatingStateToIndex("move_perfect")])
    SYNC_PROP(move_awesome, mRatingStates[RatingStateToIndex("move_awesome")])
    SYNC_PROP(move_ok, mRatingStates[RatingStateToIndex("move_ok")])
    SYNC_PROP(move_bad, mRatingStates[RatingStateToIndex("move_bad")])
    SYNC_PROP(super_perfect_threshold, mThresholds[kMoveRatingSuperPerfect])
    SYNC_PROP(perfect_threshold, mThresholds[kMoveRatingPerfect])
    SYNC_PROP(awesome_threshold, mThresholds[kMoveRatingAwesome])
    SYNC_PROP(ok_threshold, mThresholds[kMoveRatingOk])
    SYNC_PROP(super_perfect_override, mOverrides[kMoveRatingSuperPerfect])
    SYNC_PROP(perfect_override, mOverrides[kMoveRatingPerfect])
    SYNC_PROP(awesome_override, mOverrides[kMoveRatingAwesome])
    SYNC_PROP(ok_override, mOverrides[kMoveRatingOk])
    SYNC_PROP(shoulder_displacements, mShoulderDisplacements)
    SYNC_PROP(confusability_id, mConfusabilityID.mCRC)
    SYNC_PROP_SET(confusability_count, (int)mConfusabilities.size(), )
    SYNC_SUPERCLASS(RndPropAnim)
END_PROPSYNCS

BEGIN_SAVES(HamMove)
    SAVE_REVS(50, 0)
    SAVE_SUPERCLASS(RndPropAnim)
    bs << mMirror;
    bs << mTex;
    bs << mScored;
    bs << mFinalPose;
    bs << mLocalizedNames.size();
    for (int i = 0; i < mLocalizedNames.size(); i++) {
        bs << mLocalizedNames[i].mLanguage;
        bs << mLocalizedNames[i].mName;
    }
    bs << mTexState;
    int numFrames = mMoveFrames.size();
    bs << numFrames;
    for (int i = 0; i < numFrames; i++) {
        mMoveFrames[i].Save(bs);
    }
    bs << mParadiddle;
    bs << mSuppressGuide;
    bs << mSuppressPracticeOptions;
    bs << mOmitMinigame;
    bs << mRatingStates;
    bs << mShoulderDisplacements;
    for (int i = 0; i < kNumMoveRatings; i++) {
        bs << mThresholds[i];
        bs << mOverrides[i];
    }
    bs << mConfusabilities;
    bs << mDifficulty;
    bs << mDancerSeq;
    bs << mConfusabilityID;
END_SAVES

BEGIN_COPYS(HamMove)
    COPY_SUPERCLASS(RndPropAnim)
    CREATE_COPY(HamMove)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMirror)
        COPY_MEMBER(mLocalizedNames)
        COPY_MEMBER(mTex)
        COPY_MEMBER(mSmallTex)
        COPY_MEMBER(mScored)
        COPY_MEMBER(mFinalPose)
        FOREACH (it, mPropKeys) {
            (*it)->SetTarget(this);
        }
        COPY_MEMBER(mTexState)
        COPY_MEMBER(mMoveFrames)
        COPY_MEMBER(mParadiddle)
        COPY_MEMBER(mSuppressGuide)
        COPY_MEMBER(mSuppressPracticeOptions)
        COPY_MEMBER(mOmitMinigame)
        COPY_MEMBER(mRatingStates)
        COPY_MEMBER(mShoulderDisplacements)
        for (int i = 0; i < kNumMoveRatings; i++) {
            COPY_MEMBER(mThresholds[i])
            COPY_MEMBER(mOverrides[i])
        }
        COPY_MEMBER(mConfusabilities)
        COPY_MEMBER(mConfusabilityID)
        COPY_MEMBER(mDifficulty)
        COPY_MEMBER(mDancerSeq)
        if (ty == kCopyDeep && Dir()) {
            if (mTex && mTex->Dir() != Dir()) {
                RndTex *tex = Dir()->Find<RndTex>(mTex->Name(), false);
                if (tex) {
                    mTex = tex;
                }
            }
            if (mSmallTex && mSmallTex->Dir() != Dir()) {
                RndTex *tex = Dir()->Find<RndTex>(mSmallTex->Name(), false);
                if (tex) {
                    mSmallTex = tex;
                }
            }
            if (mMirror && mMirror->Dir() != Dir()) {
                HamMove *mirror = Dir()->Find<HamMove>(mMirror->Name(), false);
                if (mirror) {
                    mMirror = mirror;
                }
            }
            if (mDancerSeq && mDancerSeq->Dir() != Dir()) {
                DancerSequence *seq =
                    Dir()->Find<DancerSequence>(mDancerSeq->Name(), false);
                if (seq) {
                    mDancerSeq = seq;
                }
            }
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(50, 0)

BEGIN_LOADS(HamMove)
    LOAD_REVS(bs)
    ASSERT_REVS(50, 0)
    if (d.rev > 3) {
        RndPropAnim::Load(bs);
    } else {
        Hmx::Object::Load(bs);
    }
    if (d.rev > 7) {
        d >> mMirror;
    }
    if (d.rev < 5) {
        String str;
        d >> str;
    }
    ObjPtr<RndTex> texPtr(this);
    d >> texPtr;
    SetTexture(texPtr);
    if (d.rev > 1) {
        d >> mScored;
    }
    if (d.rev > 0x13) {
        d >> mFinalPose;
    }
    if (d.rev > 2 && d.rev < 0xB) {
        ObjPtr<RndTex> tex(this);
        d >> tex;
    }
    if (d.rev < 0x10) {
        if (d.rev > 6) {
            ObjPtrVec<Hmx::Object> objs(this);
            d >> objs;
        } else if (d.rev > 5) {
            String str;
            d >> str;
        } else {
            MILO_NOTIFY("HamMove is older than version 6, need to resave this file");
        }
    }
    if (d.rev > 4) {
        LocalizedName name;
        int count;
        d >> count;
        for (int i = 0; i < count; i++) {
            d >> name.mLanguage;
            d >> name.mName;
            if (!unkd0) {
                SetName(name.mLanguage, name.mName.c_str());
            }
        }
    }
    if (d.rev < 4) {
        MILO_NOTIFY("Can't load version older than 4");
    }
    if (d.rev < 0x27) {
        ObjPtr<SkeletonClip> clipPtr(this);
        if (d.rev > 8) {
            d >> clipPtr;
        }
        if (d.rev > 9) {
            d >> clipPtr;
            d >> clipPtr;
        }
    }
    if (d.rev > 0xB) {
        int x;
        d >> x;
        mTexState = (TexState)x;
    }
    if (d.rev > 0xC) {
        int count;
        d >> count;
        mMoveFrames.resize(count);
        for (int i = 0; i < count; i++) {
            mMoveFrames[i].Load(d);
        }
    }
    if (d.rev > 0xE && d.rev < 0x2A) {
        std::list<Symbol> syms;
        d >> syms;
    }
    if (d.rev > 0x11) {
        d >> mParadiddle;
    }
    if (d.rev > 0x14) {
        d >> mSuppressGuide;
    }
    if (d.rev > 0x31) {
        d >> mSuppressPracticeOptions;
    }
    if (d.rev > 0x21) {
        bool omit;
        d >> omit;
        if (!unkd0) {
            mOmitMinigame = omit;
        }
    }
    if (d.rev > 0x15) {
        d >> mRatingStates;
    }
    if (d.rev < 0x1A) {
        for (int i = 0; i < mRatingStates.size(); i++) {
            mRatingStates[i] = 0;
        }
    }
    if (d.rev > 0x1A) {
        d >> mShoulderDisplacements;
    }
    if (d.rev > 0x23) {
        for (int i = 0; i < kNumMoveRatings; i++) {
            d >> mThresholds[i];
            d >> mOverrides[i];
        }
    }
    if (d.rev > 0x2A) {
        std::map<Hmx::CRC, float> confusabilities;
        d >> confusabilities;
        if (!unkd0) {
            mConfusabilities = confusabilities;
        }
    }
    if (d.rev > 0x2E) {
        int diff;
        d >> diff;
        if (!unkd0) {
            mDifficulty = (Difficulty)diff;
        }
    }
    if (d.rev > 0x2F) {
        d >> mDancerSeq;
    }
    if (d.rev > 0x30) {
        Hmx::CRC id;
        d >> id;
        if (!unkd0) {
            mConfusabilityID = id;
        }
    }
    unkd0 = false;
END_LOADS

void HamMove::SetFrame(float frame, float blend) {
    RndPropAnim::SetFrame(frame, blend);
    if (TheLoadMgr.EditMode()) {
        MoveDir *dir = dynamic_cast<MoveDir *>(Dir());
        if (dir) {
            if (dir->CurrentMove(0) != this) {
                dir->SetCurrentMove(0, this);
            }
        }
    }
}

float HamMove::StartFrame() {
    if (mMirror) {
        return mMirror->StartFrame();
    }
    return RndPropAnim::StartFrame();
}

float HamMove::EndFrame() {
    if (mMirror) {
        return mMirror->EndFrame();
    }
    return RndPropAnim::EndFrame();
}

bool HamMove::IsRest() const { return !mScored; }
const char *HamMove::DisplayName() const { return mDisplayName ? mDisplayName : "NULL"; }
bool HamMove::IsFinalPose() const { return mFinalPose; }
bool HamMove::SuppressGuideGesture() const { return mSuppressGuide; }
bool HamMove::SuppressPracticeOptions() const { return mSuppressPracticeOptions; }

std::vector<MoveFrame> &HamMove::GetMoveFrames() {
    if (mMirror)
        return mMirror->mMoveFrames;
    else
        return mMoveFrames;
}

const std::vector<MoveFrame> &HamMove::GetMoveFrames() const {
    if (mMirror)
        return mMirror->mMoveFrames;
    else
        return mMoveFrames;
}

MoveMirrored HamMove::Mirrored() const { return (MoveMirrored)(mMirror != nullptr); }

void HamMove::RefreshBarks() {
    static Symbol hud_panel("hud_panel");
    DataNode &n = DataVariable(hud_panel);
    if (n.Type() == kDataObject) {
        static Message msg("add_all_barks");
        n.Obj<Hmx::Object>()->HandleType(msg);
    }
}

void HamMove::SetTexture(RndTex *tex) {
    mTex = tex;
    if (mTex) {
        mSmallTex = nullptr;
        String texName(mTex->Name());
        if (texName.find_last_of(".tex") != FixedString::npos) {
            texName.replace(texName.length() - 4, 4, "_sm.tex");
            mSmallTex = mTex->Dir()->Find<RndTex>(texName.c_str(), false);
        }
        if (!mSmallTex) {
            mSmallTex = mTex;
        }
    } else {
        mSmallTex = nullptr;
    }
}

float HamMove::FindConfusabilty(const HamMove *move) const {
    auto it = mConfusabilities.find(move->mConfusabilityID);
    if (it == mConfusabilities.end())
        return -1;
    else
        return it->second;
}

void HamMove::Update(const HamMove *other) {
    FOREACH (it, other->mLocalizedNames) {
        SetName(it->mLanguage, it->mName.c_str());
    }
    mOmitMinigame = other->mOmitMinigame;
    mConfusabilities = other->mConfusabilities;
    mConfusabilityID = other->mConfusabilityID;
    mDifficulty = other->mDifficulty;
    unkd0 = true;
}

void HamMove::SyncMirror() {
    if (mMirror == this) {
        MILO_NOTIFY("A HamMove can't mirror itself");
        mMirror = nullptr;
    }
    mMoveFrames.clear();
}

void HamMove::SetName(Symbol language, const char *name) {
    auto it = std::find(mLocalizedNames.begin(), mLocalizedNames.end(), Symbol(language));
    if (it != mLocalizedNames.end() && it) {
        it->mName = name;
        if (language == SystemLanguage()) {
            mDisplayName = it->mName.c_str();
        }
    } else {
        MILO_NOTIFY("Could not find string for language %s", language);
    }
}

float HamMove::PSNRThreshold(MoveRating r) const {
    MILO_ASSERT_RANGE(r, 0, kNumMoveRatings, 0x28f);
    float thresh = mOverrides[r];
    if (thresh == 0) {
        thresh = mThresholds[r];
    }
    return thresh;
}

float HamMove::Confusability(const HamMove *move) const {
    if (move == this)
        return 4.0f;
    else if (!move)
        return 0;
    else {
        float maxConfuse = Max(FindConfusabilty(move), move->FindConfusabilty(this));
        return Max(maxConfuse, 0.0f);
    }
}

FilterVersionType HamMove::Version() const {
    HamMove *move = (HamMove *)this;
    while (move->mMirror) {
        move = move->mMirror;
    }
    if (move->mMoveFrames.empty()) {
        return kFilterVersionHam2;
    } else {
        return move->mMoveFrames.front().Version();
    }
}

const FilterVersion *HamMove::FilterVer() const {
    return MoveDir::FindFilterVersion(Version());
}

float HamMove::ConfusabilityWithMoveDataArray(const DataArray *a) {
    float f7 = 0;
    int i3 = 0;
    float f6 = f7;
    int aSize = a->Size();
    for (int i = 0; i < aSize; i++) {
        HamMove *move = a->Obj<HamMove>(i);
        if (move != this) {
            i3++;
            float f5 = Confusability(move);
            if (f5 > f6) {
                if (f5 < 0.5f) {
                    f6 = f5;
                } else {
                    f6 = 0.5f;
                }
            }
        }
    }
    if (i3 == 0) {
        return f7;
    } else {
        return f6;
    }
}

float HamMove::AdjustNormalizedPercentToConfusability(float f1, float f2) {
    float awesomeFrac = PSNRToDetectFrac(mThresholds[kMoveRatingAwesome]);
    float perfectFrac = PSNRToDetectFrac(mThresholds[kMoveRatingPerfect]);
    float fvar1 = awesomeFrac * f2;
    if (f1 < fvar1) {
        return (0.5f / fvar1) * f1;
    } else {
        return ((f1 - fvar1) / (perfectFrac - fvar1) + 1.0f) / 2.0f;
    }
}

const std::vector<float> *HamMove::RatingOverride() const {
    if (mRatingStates.front() > 0) {
        return &mRatingStates;
    } else {
        return nullptr;
    }
}
