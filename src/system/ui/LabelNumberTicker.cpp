#include "ui/LabelNumberTicker.h"
#include "UIComponent.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Locale.h"
#include "utl/MBT.h"
#include "utl/Symbol.h"

LabelNumberTicker::LabelNumberTicker()
    : mLabel(this), mDesiredValue(0), mAnimTime(0), mAnimDelay(0), mWrapperText(gNullStr),
      mAcceleration(0), unk6c(0), unk70(0), mTickTrigger(this), mTickEvery(0) {}

LabelNumberTicker::~LabelNumberTicker() {}

BEGIN_LOADS(LabelNumberTicker)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

BEGIN_SAVES(LabelNumberTicker)
    SAVE_REVS(2, 0)
    bs << mLabel << mDesiredValue << mAnimTime << mAnimDelay << mWrapperText;
    bs << mAcceleration;
    bs << mTickTrigger << mTickEvery;
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(LabelNumberTicker)
    CREATE_COPY_AS(LabelNumberTicker, p)
    MILO_ASSERT(p, 0x2d);
    COPY_MEMBER_FROM(p, mLabel)
    COPY_MEMBER_FROM(p, mDesiredValue)
    COPY_MEMBER_FROM(p, mAnimTime)
    COPY_MEMBER_FROM(p, mAnimDelay)
    COPY_MEMBER_FROM(p, mWrapperText)
    COPY_MEMBER_FROM(p, mAcceleration)
    COPY_MEMBER_FROM(p, mTickTrigger)
    COPY_MEMBER_FROM(p, mTickEvery)
    UIComponent::Copy(p, ty);
END_COPYS

BEGIN_PROPSYNCS(LabelNumberTicker)
    SYNC_PROP_SET(label, mLabel.Ptr(), SetLabel(_val.Obj<UILabel>()))
    SYNC_PROP_SET(desired_value, mDesiredValue, SetDesiredValue(_val.Int()))
    SYNC_PROP_MODIFY(wrapper_text, mWrapperText, UpdateDisplay())
    SYNC_PROP_MODIFY(anim_time, mAnimTime, UpdateDisplay())
    SYNC_PROP_MODIFY(anim_delay, mAnimDelay, UpdateDisplay())
    SYNC_PROP(acceleration, mAcceleration)
    SYNC_PROP(tick_trigger, mTickTrigger)
    SYNC_PROP(tick_every, mTickEvery)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

void LabelNumberTicker::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    UIComponent::PostLoad(bs);
}

void LabelNumberTicker::Init() { REGISTER_OBJ_FACTORY(LabelNumberTicker) }

void LabelNumberTicker::UpdateDisplay() {
    if (mLabel) {
        if (mWrapperText != gNullStr) {
            mLabel->SetTokenFmt(mWrapperText, LocalizeSeparatedInt(unk70, TheLocale));
        }
    }
}

void LabelNumberTicker::SetLabel(UILabel *l) {
    mLabel = l;
    UpdateDisplay();
}

void LabelNumberTicker::SetDesiredValue(int i) {
    unk6c = unk70;
    mDesiredValue = i;
    if (mAnimDelay + mAnimTime <= 0.0f)
        unk70 = i;
    else {
        mTimer.Reset();
        mTimer.Start();
    }
    if (mTickTrigger && i > 0) {
        mTickTrigger->Trigger();
    }
    UpdateDisplay();
}

void LabelNumberTicker::CountUp() {
    int val = mDesiredValue;
    unk70 = 0;
    mDesiredValue = 0;
    UpdateDisplay();
    SetDesiredValue(val);
}

void LabelNumberTicker::CountUpFromCurrentValue() {
    int val = mDesiredValue;
    if (unk70 >= val) {
        unk70 = 0;
        mDesiredValue = 0;
        UpdateDisplay();
    }
    SetDesiredValue(val);
}

void LabelNumberTicker::Enter() {
    UIComponent::Enter();
    UpdateDisplay();
}

void LabelNumberTicker::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0)
    bs >> mLabel;
    bs >> mDesiredValue;
    bs >> mAnimTime;
    bs >> mAnimDelay;
    bs >> mWrapperText;
    if (d.rev >= 1)
        bs >> mAcceleration;
    if (2 <= d.rev) {
        bs >> mTickTrigger;
        bs >> mTickEvery;
    }
    UIComponent::PreLoad(bs);
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void LabelNumberTicker::SnapToValue(int i) {
    unk70 = i;
    mDesiredValue = i;
    UpdateDisplay();
}

void LabelNumberTicker::Poll() {
    UIComponent::Poll();
    if (mTimer.Running()) {
        float split = mTimer.SplitMs();
        float animtime = mAnimTime * 1000.0f;
        float animdelay = mAnimDelay * 1000.0f;
        float animsum = animdelay + animtime;
        if (split >= animdelay) {
            float quotient = (split - animdelay) / animtime;
            quotient *= std::pow(quotient, mAcceleration);
            int somenum = unk6c + (int)(quotient * (mDesiredValue - unk6c));
            if (mTickTrigger && mTickEvery != 0) {
                if ((somenum / mTickEvery) > (unk70 / mTickEvery)) {
                    mTickTrigger->Trigger();
                }
            }
            unk70 = somenum;
            if (unk70 == mDesiredValue || (split >= animsum)) {
                unk70 = mDesiredValue;
                mTimer.Stop();
            }
        }
        UpdateDisplay();
    }
}

BEGIN_HANDLERS(LabelNumberTicker)
    HANDLE_ACTION(snap_to_value, SnapToValue(_msg->Int(2)))
    HANDLE_ACTION(count_up, CountUp())
    HANDLE_ACTION(count_up_from_current, CountUpFromCurrentValue())
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS
