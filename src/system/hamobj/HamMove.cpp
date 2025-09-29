#include "hamobj/HamMove.h"
#include "FilterVersion.h"
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

float HamMove::sMinFrameDistBeats = 0.2;

HamMove::HamMove()
    : mMirror(this), mTex(this), mSmallTex(this), mTexState(kTexNormal), mScored(true),
      mParadiddle(false), mFinalPose(false), mSuppressGuide(false),
      mSuppressPracticeOptions(false), mOmitMinigame(false), mDisplayName(nullptr),
      mDifficulty(kDifficultyExpert), mShoulderDisplacements(false), unkd0(0),
      mDancerSeq(this) {
    SetRate(k480_fpb);
    SystemLanguage();
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
    for (int i = 4; i > 0; i--) {
        mRatingStates.push_back(0);
        mThresholds[i - 1] = i * 25.0f;
        mOverrides[i - 1] = 0;
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
    for (int i = 0; i < 4; i++) {
        bs << mThresholds[i];
        bs << mOverrides[i];
    }
    bs << mConfusabilities;
    bs << mDifficulty;
    bs << mDancerSeq;
    bs << mConfusabilityID;
END_SAVES

bool HamMove::IsRest() const { return !mScored; }
const char *HamMove::DisplayName() const { return mDisplayName ? mDisplayName : "NULL"; }
bool HamMove::IsFinalPose() const { return mFinalPose; }
bool HamMove::SuppressGuideGesture() const { return mSuppressGuide; }
bool HamMove::SuppressPracticeOptions() const { return mSuppressPracticeOptions; }

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

void MoveFrame::Save(BinStream &bs) const {
    bs << mBeat;
    bs << unk4;
    bs << 16;
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

void MoveFrame::Load(BinStreamRev &bs) {
    bs >> mBeat;
    if (bs.rev < 14) {
        MILO_FAIL("Versions less than 14 no longer supported");
    }
    if (bs.rev > 0x2B) {
        bs >> unk4;
    } else
        unk4 = -1;
}

const Ham1NodeWeight &
MoveFrame::NodeWeightHam1(int node, MoveMode mode, MoveMirrored mirror) const {
    MILO_ASSERT((0) <= (mode) && (mode) < (kNumMoveModes), 0x20);
    MILO_ASSERT((0) <= (mirror) && (mirror) < (kNumMoveMirrored), 0x21);
    MILO_ASSERT((0) <= (node) && (node) < (kNumHam1Nodes), 0x22);
    return mHam1NodeWeights[mode][mirror][node];
}

const Ham2FrameWeight &MoveFrame::FrameWeight(MoveMirrored mirror) const {
    MILO_ASSERT(mFrameWeights, 0x37);
    MILO_ASSERT((0) <= (mirror) && (mirror) < (kNumMoveMirrored), 0x38);
    return mFrameWeights[mirror];
}

const Vector3 &MoveFrame::NodeWeight(int node, MoveMirrored mirror) const {
    MILO_ASSERT((0) <= (mirror) && (mirror) < (kNumMoveMirrored), 0x28);
    MILO_ASSERT((0) <= (node) && (node) < (FilterVersion::NumHam2Nodes()), 0x29);
    return mNodeWeights[mirror][node];
}

const Vector3 &MoveFrame::NodeInverseScale(int node, MoveMirrored mirror) const {
    MILO_ASSERT((0) <= (mirror) && (mirror) < (kNumMoveMirrored), 0x30);
    MILO_ASSERT((0) <= (node) && (node) < (FilterVersion::NumHam2Nodes()), 0x31);
    return mNodesInverseScale[mirror][node];
}

void MoveFrame::SetNodeScale(int node, MoveMirrored mirror, const Vector3 &v) {
    MILO_ASSERT((0) <= (mirror) && (mirror) < (kNumMoveMirrored), 0x6D);
    MILO_ASSERT((0) <= (node) && (node) < (FilterVersion::NumHam2Nodes()), 0x6E);
    for (int i = 0; i < 3; i++) {
        float s = v[i];
        MILO_ASSERT(s > 0, 0x72);
        mNodeScales[mirror][node][i] = s;
        mNodesInverseScale[mirror][node][i] = 1.0f / s;
    }
}

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
    std::map<Hmx::CRC, float>::const_iterator it =
        mConfusabilities.find(move->mConfusabilityID);
    if (it == mConfusabilities.end())
        return -1;
    else
        return it->second;
}

void HamMove::Update(const HamMove *other) {
    for (std::vector<LocalizedName>::const_iterator it = other->mLocalizedNames.begin();
         it != other->mLocalizedNames.end();
         ++it) {
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
    std::vector<LocalizedName>::iterator it =
        std::find(mLocalizedNames.begin(), mLocalizedNames.end(), language);
    if (it != mLocalizedNames.end() && it) {
        it->mName = name;
        if (language == SystemLanguage()) {
            mDisplayName = it->mName.c_str();
        }
    } else {
        MILO_NOTIFY("Could not find string for language %s", language);
    }
}
