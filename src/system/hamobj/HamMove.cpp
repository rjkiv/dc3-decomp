#include "hamobj/HamMove.h"
#include "hamobj/ScoreUtl.h"
#include "hamobj/Difficulty.h"
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
    } else if (d.rev > 0x18) {
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

// class BinStream & __cdecl operator<<(class BinStream &, struct Ham2FrameWeight const &)
// class BinStream & __cdecl operator>>(class BinStreamRev &, struct Ham2FrameWeight &)
