#include "hamobj/HamListRibbon.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "utl/BinStream.h"

HamListRibbon::HamListRibbon()
    : mScrollAnims(this), mTestMode(0), mTestNumDisplay(4), mTestSelectedIndex(0),
      mSpacing(25), mMode(kRibbonSlide), mTestEntering(0), mPaddedSize(0),
      mPaddedSpacing(29), unk26c(0), mSwellAnim(this), mSlideAnim(this),
      mSelectAnim(this), mSelectToggleAnim(this), mSelectInactiveAnim(this),
      mSelectAllAnim(this), mDisengageAnim(this), mEnterAnim(this),
      mLabelPlaceholder(this), mHighlightSounds(this), mSelectSounds(this),
      mEnterFlow(this), mSlideSound(this), mSlideSoundAnim(this), mScrollSound(this),
      mScrollSoundAnim(this) {}

BEGIN_HANDLERS(HamListRibbon)
    HANDLE(enter_blacklight_mode, OnEnterBlacklightMode)
    HANDLE(exit_blacklight_mode, OnExitBlacklightMode)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(HamListRibbon::ScrollAnims)
    SYNC_PROP(scroll_anim, o.mScrollAnim)
    SYNC_PROP(scroll_active, o.mScrollActive)
    SYNC_PROP(scroll_fade, o.mScrollFade)
    SYNC_PROP(scroll_faded, o.mScrollFaded)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamListRibbon)
    SYNC_PROP(test_mode, mTestMode)
    SYNC_PROP(test_entering, mTestEntering)
    SYNC_PROP(test_num_display, mTestNumDisplay)
    SYNC_PROP(test_selected_index, mTestSelectedIndex)
    SYNC_PROP(spacing, mSpacing)
    SYNC_PROP(padded_size, mPaddedSize)
    SYNC_PROP(padded_spacing, mPaddedSpacing)
    SYNC_PROP_SET(mode, (int &)mMode, mMode = (RibbonMode)_val.Int())
    SYNC_PROP(swell_anim, mSwellAnim)
    SYNC_PROP(slide_anim, mSlideAnim)
    SYNC_PROP(select_anim, mSelectAnim)
    SYNC_PROP(select_inactive_anim, mSelectInactiveAnim)
    SYNC_PROP(select_all_anim, mSelectAllAnim)
    SYNC_PROP(select_toggle_anim, mSelectToggleAnim)
    SYNC_PROP(enter_flow, mEnterFlow)
    SYNC_PROP(enter_anim, mEnterAnim)
    SYNC_PROP(disengage_anim, mDisengageAnim)
    SYNC_PROP(scroll_anims, mScrollAnims)
    SYNC_PROP(label_placeholder, mLabelPlaceholder)
    SYNC_PROP(highlight_sounds, mHighlightSounds)
    SYNC_PROP(select_sounds, mSelectSounds)
    SYNC_PROP(slide_sound, mSlideSound)
    SYNC_PROP(slide_sound_anim, mSlideSoundAnim)
    SYNC_PROP(scroll_sound, mScrollSound)
    SYNC_PROP(scroll_sound_anim, mScrollSoundAnim)
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(HamListRibbon)
    SAVE_REVS(11, 0)
    SAVE_SUPERCLASS(RndDir)
    bs << mSpacing;
    bs << mSwellAnim;
    bs << mSlideAnim;
    bs << mSelectAnim;
    bs << mSelectInactiveAnim;
    bs << mSelectAllAnim;
    bs << mLabelPlaceholder;
    mScrollAnims.Save(bs);
    bs << mDisengageAnim;
    bs << mSlideSound;
    bs << mSlideSoundAnim;
    bs << mScrollSound;
    bs << mScrollSoundAnim;
    bs << mEnterFlow;
    bs << mEnterAnim;
    bs << mPaddedSize;
    bs << mPaddedSpacing;
    bs << mHighlightSounds;
    bs << mSelectSounds;
    bs << mSelectToggleAnim;
END_SAVES

BEGIN_COPYS(HamListRibbon)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY(HamListRibbon)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMode)
        COPY_MEMBER(mSpacing)
        COPY_MEMBER(mSwellAnim)
        COPY_MEMBER(mSlideAnim)
        COPY_MEMBER(mSelectAnim)
        COPY_MEMBER(mSelectInactiveAnim)
        COPY_MEMBER(mSelectAllAnim)
        COPY_MEMBER(mSelectToggleAnim)
        COPY_MEMBER(mEnterFlow)
        COPY_MEMBER(mEnterAnim)
        COPY_MEMBER(mLabelPlaceholder)
        COPY_MEMBER(mScrollAnims)
        COPY_MEMBER(mDisengageAnim)
        COPY_MEMBER(mHighlightSounds)
        COPY_MEMBER(mSelectSounds)
        COPY_MEMBER(mSlideSound)
        COPY_MEMBER(mSlideSoundAnim)
        COPY_MEMBER(mScrollSound)
        COPY_MEMBER(mScrollSoundAnim)
        COPY_MEMBER(mPaddedSize)
        COPY_MEMBER(mPaddedSpacing)
    END_COPYING_MEMBERS
END_COPYS

void HamListRibbon::ScrollAnims::SetScrollFrame(float frame) {
    if (mScrollAnim)
        mScrollAnim->SetFrame(frame, 1);
}

void HamListRibbon::ScrollAnims::SetAnims(int i1) {
    if (mScrollAnim) {
        float frame = mScrollAnim->GetFrame();
        if (i1 == 0) {
            if (mScrollFade)
                mScrollFade->SetFrame(1 - frame, 1);
        } else if (i1 > 0 && i1 < 4) {
            if (mScrollActive)
                mScrollActive->SetFrame(frame, 1);
        } else if (i1 == 4) {
            if (mScrollFade)
                mScrollFade->SetFrame(frame, 1);
        } else if (mScrollFaded)
            mScrollFaded->SetFrame(frame, 1);
    }
}

void HamListRibbon::ScrollAnims::Save(BinStream &bs) const {
    bs << mScrollAnim;
    bs << mScrollActive;
    bs << mScrollFade;
    bs << mScrollFaded;
}

void HamListRibbon::ScrollAnims::Load(BinStreamRev &bs) {
    bs >> mScrollAnim;
    bs >> mScrollActive;
    bs >> mScrollFade;
    bs >> mScrollFaded;
}

void HamListRibbon::HandleEnter() {
    if (mEnterFlow)
        mEnterFlow->Activate();
    ResetAnims(true);
}

void HamListRibbon::OnSelectDone() { ResetAnims(true); }

DataNode HamListRibbon::OnEnterBlacklightMode(const DataArray *a) {
    Flow *flow = DataDir()->Find<Flow>("activate_blacklight.flow", false);
    if (flow)
        flow->Activate();
    return 0;
}

DataNode HamListRibbon::OnExitBlacklightMode(const DataArray *a) {
    Flow *flow;
    if (a->Int(2) == 0) {
        flow = DataDir()->Find<Flow>("deactivate_blacklight.flow", false);
    } else {
        flow = DataDir()->Find<Flow>("deactivate_blacklight_immediate.flow", false);
    }
    if (flow) {
        flow->Activate();
    }
    return 0;
}

void HamListRibbon::PlayHighlightSound(int idx) {
    if (idx >= mHighlightSounds.size() - 1)
        return;
    else
        mHighlightSounds[idx]->Activate();
}

void HamListRibbon::PlaySelectSound(int idx) {
    if (idx < mSelectSounds.size()) {
        mSelectSounds[idx]->Activate();
    }
}
