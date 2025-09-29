#include "hamobj/HamMove.h"
#include "HamMove.h"
#include "ScoreUtl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Tex.h"
#include "utl/Loader.h"

HamMove::HamMove()
    : mMirror(this), mTex(this), mSmallTex(this), mTexState(kTexNormal), mScored(true),
      mParadiddle(false), mFinalPose(false), mSuppressGuide(false),
      mSuppressPracticeOptions(false), mOmitMinigame(false), mDisplayName(nullptr),
      unk94(2), mShoulderDisplacements(false), unkd0(0), mConfusabilityID(0),
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
    SYNC_PROP_SET(difficulty, unk94, unk94 = _val.Int())
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
    SYNC_PROP(confusability_id, mConfusabilityID)
    SYNC_PROP_SET(confusability_count, (int)mConfusabilities.size(), )
    SYNC_SUPERCLASS(RndPropAnim)
END_PROPSYNCS
