#include "hamobj/HamListRibbon.h"
#include "hamobj/HamLabel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Text.h"
#include "utl/BinStream.h"

#pragma region ScrollAnims

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

#pragma endregion
#pragma region HamListRibbon

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

INIT_REVS(11, 0)

void HamListRibbon::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0xB, 0)
    RndDir::PreLoad(d.stream);
    d.PushRev(this);
}

void HamListRibbon::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    RndDir::PostLoad(d.stream);
    d >> mSpacing;
    d >> mSwellAnim;
    d >> mSlideAnim;
    d >> mSelectAnim;
    d >> mSelectInactiveAnim;
    d >> mSelectAllAnim;
    d >> mLabelPlaceholder;
    if (d.rev >= 2) {
        mScrollAnims.Load(d);
    }
    if (d.rev >= 3) {
        d >> mDisengageAnim;
    }
    if (d.rev >= 4) {
        if (d.rev < 9) {
            Symbol s;
            int num;
            d >> num;
            for (int i = 0; i < num; i++) {
                d >> s;
            }
            d >> num;
            for (int i = 0; i < num; i++) {
                d >> s;
            }
        }
        if (d.rev == 4) {
            Symbol s;
            d >> s;
            d >> s;
        }
    }
    if (d.rev >= 5) {
        d >> mSlideSound;
        d >> mSlideSoundAnim;
        d >> mScrollSound;
        d >> mScrollSoundAnim;
    }
    if (d.rev >= 10) {
        d >> mEnterFlow;
    }
    if (d.rev >= 6) {
        d >> mEnterAnim;
    }
    if (d.rev >= 7) {
        d >> mPaddedSize;
    }
    if (d.rev >= 8) {
        d >> mPaddedSpacing;
    }
    if (d.rev >= 9) {
        d >> mHighlightSounds;
        d >> mSelectSounds;
    }
    if (d.rev >= 11) {
        d >> mSelectToggleAnim;
    }
}

void HamListRibbon::DrawShowing() {
    if (!mTestMode) {
        RndDir::DrawShowing();
    } else {
        std::vector<HamListRibbonDrawState> drawStates(mTestNumDisplay);
        for (int i = 0; i < mTestNumDisplay; i++) {
            if (i == mTestSelectedIndex) {
                drawStates[i].unk14 = true;
                if (mMode == kRibbonSwell && !mTestEntering) {
                    float frame = GetFrame();
                    drawStates[i].unk0.SetParams(frame, frame, 0);
                } else {
                    drawStates[i].unk0.SetParams(1, 1, 0);
                }
            } else {
                drawStates[i].unk14 = false;
                drawStates[i].unk0.SetParams(0, 0, 0);
            }
        }
        Transform xfm = WorldXfm();
        Draw(xfm, drawStates, true, false);
    }
}

float HamListRibbon::StartFrame() {
    if (mTestEntering && mEnterAnim) {
        return mEnterAnim->StartFrame();
    } else {
        switch (mMode) {
        case kRibbonSwell:
            if (mSwellAnim) {
                return mSwellAnim->StartFrame();
            } else {
                return 0;
            }
        case kRibbonSlide:
            if (mSlideAnim) {
                return mSlideAnim->StartFrame();
            } else {
                return 0;
            }
        case kRibbonSelect:
            if (unk26c && mSelectToggleAnim) {
                return mSelectToggleAnim->StartFrame();
            } else if (mSelectAnim || mSelectAllAnim) {
                if (mSelectAnim && !mSelectAllAnim) {
                    return mSelectAnim->StartFrame();
                } else if (!mSelectAnim && mSelectAllAnim) {
                    return mSelectAllAnim->StartFrame();
                } else if (mSelectAnim && mSelectAllAnim) {
                    return Min(mSelectAnim->StartFrame(), mSelectAllAnim->StartFrame());
                }
            }
            return 0;
        default:
            return 0;
        }
    }
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
    int numSounds = mHighlightSounds.size();
    if (numSounds != 0) {
        mHighlightSounds[Min(idx, numSounds - 1)]->Activate();
    }
}

void HamListRibbon::PlaySelectSound(int idx) {
    int numSounds = mSelectSounds.size();
    if (numSounds != 0 && idx >= 0) {
        mSelectSounds[Min(idx, numSounds - 1)]->Activate();
    }
}

bool HamListRibbon::IsScrollable(int i1) const { return i1 > 6; }

void HamListRibbon::ResetAnims(bool b1) {
    if (mSelectInactiveAnim && (mSelectInactiveAnim->GetFrame() != 0 || b1)) {
        mSelectInactiveAnim->SetFrame(0, 1);
    }
    if (mSelectAnim && (mSelectAnim->GetFrame() != 0 || b1)) {
        mSelectAnim->SetFrame(0, 1);
    }
    if (mSelectToggleAnim && (mSelectToggleAnim->GetFrame() != 0 || b1)) {
        mSelectToggleAnim->SetFrame(0, 1);
    }
    if (mSlideAnim && (mSlideAnim->GetFrame() != 0 || b1)) {
        mSlideAnim->SetFrame(0, 1);
    }
    if (mSwellAnim && (mSwellAnim->GetFrame() != 0 || b1)) {
        mSwellAnim->SetFrame(0, 1);
    }
}

void HamListRibbon::SetAnims(bool b1, float f2) {
    if (mTestEntering)
        return;
    if (mSwellAnim) {
        mSwellAnim->SetFrame(f2, 1);
    }
    if (b1) {
        if (mMode == 1 && mSlideAnim) {
            mSlideAnim->SetFrame(GetFrame(), 1);
        }
        if (mMode == 2) {
            if (unk26c && mSelectToggleAnim) {
                mSelectToggleAnim->SetFrame(GetFrame(), 1);
            } else if (mSelectAnim) {
                mSelectAnim->SetFrame(GetFrame(), 1);
            }
        }
    } else {
        if (mMode == 2 && !unk26c && mSelectInactiveAnim) {
            mSelectInactiveAnim->SetFrame(GetFrame(), 1);
        }
    }
}

void HamListRibbon::SetDisengageFrame(float f1) {
    if (mDisengageAnim) {
        mDisengageAnim->SetFrame(f1, 1);
    }
}

float HamListRibbon::GetLabelTotalAlpha() const {
    float ret = 1;
    for (unsigned int i = 0; i < mLabelPlaceholder->NumStyles(); i++) {
        ret *= mLabelPlaceholder->Style(i).GetAlpha();
    }
    return ret;
}
