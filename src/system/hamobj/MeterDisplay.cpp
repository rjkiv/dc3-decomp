#include "hamobj/MeterDisplay.h"
#include "MeterDisplay.h"
#include "hamobj/HamLabel.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "ui/UIComponent.h"
#include "utl/BinStream.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

MeterDisplay::MeterDisplay()
    : mMeterAnim(0), mAnimPeriod(0), unk4c(0), unk50(-1), unk54(0), mShowText(0),
      mPercentageText(0), mHideDenominator(0), mWrapperText(gNullStr), mCurrentValue(0),
      mMaxValue(0), mResourceDir(this) {}

MeterDisplay::~MeterDisplay() { delete unk54; }

BEGIN_HANDLERS(MeterDisplay)
    HANDLE_ACTION(animate_to_value, AnimateToValue(_msg->Int(2), _msg->Int(3)))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(MeterDisplay)
    SYNC_PROP_MODIFY(show_text, mShowText, UpdateDisplay())
    SYNC_PROP_MODIFY(percentage_text, mPercentageText, UpdateDisplay())
    SYNC_PROP_MODIFY(hide_denominator, mHideDenominator, UpdateDisplay())
    SYNC_PROP_MODIFY(wrapper_text, mWrapperText, UpdateDisplay())
    SYNC_PROP_MODIFY(current_value, mCurrentValue, UpdateDisplay())
    SYNC_PROP_MODIFY(max_value, mMaxValue, UpdateDisplay())
    SYNC_PROP(anim_period, mAnimPeriod)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(MeterDisplay)
    SAVE_REVS(4, 0)
    bs << mShowText << mCurrentValue << mMaxValue << mPercentageText << mAnimPeriod
       << mHideDenominator << mWrapperText;
    bs << mResourceDir;
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(MeterDisplay)
    CREATE_COPY_AS(MeterDisplay, p)
    MILO_ASSERT(p, 0x31);
    COPY_MEMBER_FROM(p, mShowText)
    COPY_MEMBER_FROM(p, mPercentageText)
    COPY_MEMBER_FROM(p, mHideDenominator)
    COPY_MEMBER_FROM(p, mWrapperText)
    COPY_MEMBER_FROM(p, mCurrentValue)
    COPY_MEMBER_FROM(p, mMaxValue)
    COPY_MEMBER_FROM(p, mAnimPeriod)
    COPY_MEMBER_FROM(p, mResourceDir)
    COPY_SUPERCLASS_FROM(UIComponent, p)
    Update();
END_COPYS

BEGIN_LOADS(MeterDisplay)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(4, 0)

void MeterDisplay::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    d >> mShowText;
    if (d.rev >= 1) {
        d >> mCurrentValue;
        d >> mMaxValue;
    }
    if (d.rev >= 2) {
        d >> mPercentageText;
    }
    if (d.rev >= 3) {
        d >> mAnimPeriod;
    }
    if (d.rev >= 4) {
        d >> mHideDenominator;
        d >> mWrapperText;
    }
    d >> mResourceDir;
    UIComponent::PreLoad(d.stream);
}

void MeterDisplay::PostLoad(BinStream &bs) {
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
}

void MeterDisplay::DrawShowing() {
    if (mResourceDir) {
        float f8 = 0;
        if (mMaxValue > 0) {
            f8 = (float)mCurrentValue / (float)mMaxValue;
            float f6 = TheTaskMgr.UISeconds() - unk4c;
            if (mAnimPeriod > 0 && unk50 >= 0 && f6 > 0) {
                if (f6 < mAnimPeriod) {
                    f8 = ((f6 / mAnimPeriod) * (float)(unk50 - mCurrentValue)
                          + (float)mCurrentValue)
                        / (float)mMaxValue;
                } else {
                    mCurrentValue = unk50;
                    unk50 = -1;
                    f8 = (float)mCurrentValue / (float)mMaxValue;
                    UpdateDisplay();
                }
            }
        }
        ClampEq(f8, 0.0f, 1.0f);
        float range = mMeterAnim->EndFrame() - mMeterAnim->StartFrame();
        mMeterAnim->SetFrame(range * f8 + mMeterAnim->StartFrame(), 1);
        mResourceDir->SetWorldXfm(WorldXfm());
        mResourceDir->Draw();
        if (mShowText) {
            if (unk54) {
                unk54->Draw();
            }
        }
    }
}

void MeterDisplay::Poll() {
    if (mResourceDir) {
        mResourceDir->Poll();
    }
    UIComponent::Poll();
}

void MeterDisplay::Enter() {
    UIComponent::Enter();
    UpdateDisplay();
}

void MeterDisplay::OldResourcePreload(BinStream &bs) {
    char name[256];
    bs.ReadString(name, 256);
    mResourceDir.SetName(name, true);
}

void MeterDisplay::Update() {
    if (mResourceDir) {
        static Symbol meter_label("meter_label");
        HamLabel *label = mResourceDir->Find<HamLabel>("meter.lbl", false);
        if (label) {
            if (!unk54) {
                unk54 = Hmx::Object::New<HamLabel>();
            }
            unk54->Copy(label, kCopyShallow);
            unk54->SetTransParent(label->TransParent(), false);
            label->SetShowing(false);
        }
        static Symbol meter_anim("meter_anim");
        mMeterAnim = mResourceDir->Find<RndAnimatable>("meter.anim");
    }
}

void MeterDisplay::Init() { REGISTER_OBJ_FACTORY(MeterDisplay); }

void MeterDisplay::AnimateToValue(int x, int y) {
    unk50 = Min(x, mMaxValue);
    unk4c = (y / 1000.0f) + TheTaskMgr.UISeconds();
}

void MeterDisplay::UpdateDisplay() {
    if (unk54) {
        static Symbol meter_progress_generic_wrapper("meter_progress_generic_wrapper");
        String str;
        if (mPercentageText) {
            static Symbol meter_progress_percent("meter_progress_percent");
            float f1 = 0;
            if (mMaxValue > 0) {
                f1 = (float)mCurrentValue / (float)mMaxValue;
            }
            int perc = f1 * 100.0f;
            str = MakeString(Localize(meter_progress_percent, nullptr, TheLocale), perc);
        } else if (mHideDenominator) {
            static Symbol meter_progress_no_denominator("meter_progress_no_denominator");
            String localized(LocalizeSeparatedInt(mCurrentValue, TheLocale));
            str = MakeString(
                Localize(meter_progress_no_denominator, nullptr, TheLocale), localized
            );
        } else {
            static Symbol meter_progress("meter_progress");
            String curLocalized(LocalizeSeparatedInt(mCurrentValue, TheLocale));
            String maxLocalized(LocalizeSeparatedInt(mMaxValue, TheLocale));
            str = MakeString(
                Localize(meter_progress, nullptr, TheLocale), curLocalized, maxLocalized
            );
        }
        unk54->SetPrelocalizedString(String(""));
        if (mWrapperText != gNullStr) {
            unk54->SetTokenFmt(mWrapperText, str);
        } else {
            unk54->SetTokenFmt(meter_progress_generic_wrapper, str);
        }
        unk54->SetShowing(true);
    }
}
