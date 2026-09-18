#include "ui/UIListState.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "ui/UIListProvider.h"
#include "utl/Loader.h"
#include "utl/Std.h"

UIListState::UIListState(UIListProvider *provider, UIListStateCallback *callback)
    : mCircular(0), mNumDisplay(5), mGridSpan(1), mSpeed(0.25f), mMinDisplay(0),
      mScrollPastMinDisplay(0), mMaxDisplay(-1), mScrollPastMaxDisplay(1),
      mProvider(provider), mFirstShowing(0), mTargetShowing(0), mSelectedDisplay(0),
      mStepPercent(0.0f), mStepTime(-1.0f), mCallback(callback) {}

int UIListState::SelectedDisplay() const {
    if (mCircular) {
        return mMinDisplay;
    } else {
        return mSelectedDisplay;
    }
}

void UIListState::SetNumDisplay(int num, bool b) {
    MILO_ASSERT(num > 0, 0x139);
    mNumDisplay = num;
    if (b) {
        SetSelected(0, -1, true);
    }
}

void UIListState::SetGridSpan(int span, bool b) {
    MILO_ASSERT(span > 0, 0x141);
    mGridSpan = span;
    if (b) {
        SetSelected(0, -1, true);
    }
}

void UIListState::SetSelected(int i1, int i2, bool b3) {
    int i7 = WrapShowing(i1);
    if (b3) {
        int i;
        for (i = i7; !mProvider->IsActive(Showing2Data(i));) {
            i++;
            if (Showing2Data(i) == Showing2Data(i7))
                break;
        }
        i7 = WrapShowing(i);
    }
    if (mCircular) {
        mFirstShowing = WrapShowing(i7 - mMinDisplay);
    } else {
        if (i2 != -1) {
            mFirstShowing = i2;
        } else {
            mFirstShowing = Max(0, mScrollPastMinDisplay ? i7 : i7 - mMinDisplay);
        }
        mFirstShowing = Min(mFirstShowing, MaxFirstShowing());
        mSelectedDisplay = i7 - mFirstShowing;
        if (mScrollPastMinDisplay) {
            mSelectedDisplay += mMinDisplay;
        }
    }
    mTargetShowing = mFirstShowing;
    mStepTime = -1;
    mStepPercent = 0;
}

void UIListState::SetSpeed(float speed) {
    MILO_ASSERT(speed >= 0, 0x15f);
    mSpeed = speed;
}

float UIListState::Speed() const { return mSpeed; }

void UIListState::SetMinDisplay(int min) {
    MILO_ASSERT(min >= 0, 0x149);
    mMinDisplay = min;
    mSelectedDisplay = Max(min, mSelectedDisplay);
}

void UIListState::SetMaxDisplay(int max) {
    MILO_ASSERT(max >= -1, 0x150);
    if (TheLoadMgr.EditMode()) {
        max = Clamp(-1, mNumDisplay - 1, max);
    }
    mMaxDisplay = max;
}

void UIListState::SetScrollPastMinDisplay(bool scroll) {
    mScrollPastMinDisplay = scroll;
    if (mScrollPastMinDisplay) {
        mSelectedDisplay = Max(mSelectedDisplay, mMinDisplay);
    }
}

void UIListState::SetScrollPastMaxDisplay(bool) {}

int UIListState::Selected() const { return Display2Showing(SelectedDisplay()); }

int UIListState::SelectedData() const { return Display2Data(SelectedDisplay()); }

int UIListState::CurrentScroll() const { return ScrollToTarget(mTargetShowing); }

int UIListState::WrapShowing(int i) const {
    if (NumShowing() == 0)
        return 0;
    return Mod(i, NumShowing());
}

int UIListState::Display2Showing(int i) const {
    int offset = mFirstShowing + i;
    if (mScrollPastMinDisplay && !mCircular) {
        offset = offset - mMinDisplay;
        if (offset < 0 || offset >= NumShowing())
            return -1;
    }
    return WrapShowing(offset);
}

int UIListState::Showing2Data(int i) const {
    int count = WrapShowing(i);
    FOREACH (it, mHiddenData) {
        if (*it <= count)
            count++;
    }
    return count;
}

int UIListState::NumDisplayWithData() const {
    int ret = mNumDisplay;
    if (!mCircular) {
        int num = mProvider->NumData();
        if (mScrollPastMinDisplay)
            num += mMinDisplay;
        ret = Min(ret, num);
    }
    return ret;
}

int UIListState::MaxFirstShowing() const {
    MILO_ASSERT(!mCircular, 0xE8);
    int curshowing = NumShowing();
    int maxshowing = Max(0, curshowing - mNumDisplay);
    if (mMaxDisplay != -1 && mScrollPastMaxDisplay) {
        maxshowing +=
            (Min(curshowing, mNumDisplay) - Clamp(0, mNumDisplay, mMaxDisplay)) - 1;
    }
    if (mScrollPastMinDisplay) {
        maxshowing += mMinDisplay;
    }
    return Max(0, maxshowing);
}

int UIListState::ScrollMaxDisplay() const {
    MILO_ASSERT(!mCircular, 0xF7);
    int max = Max(0, Min(NumShowing() - 1, mNumDisplay - 1));
    if (mMaxDisplay != -1) {
        max = Clamp(0, max, mMaxDisplay);
    }
    return max;
}

int UIListState::SelectedNoWrap() const {
    int i1 = mFirstShowing + SelectedDisplay();
    if (mScrollPastMinDisplay && !mCircular) {
        i1 -= mMinDisplay;
        if (i1 < 0 || i1 >= NumShowing())
            return -1;
    }
    return i1;
}

int UIListState::Display2Data(int i) const {
    int disp = Display2Showing(i);
    if (disp == -1)
        return -1;
    else
        return Showing2Data(disp);
}

int UIListState::SnappedDataForDisplay(int i2) const {
    bool b1 = (!IsScrolling() && i2 == 0) || (mTargetShowing > mFirstShowing && i2 == 0)
        || (mTargetShowing < mFirstShowing && i2 == -1);
    if (b1) {
        int data = Display2Data(i2);
        return Provider()->SnappableAtOrBeforeData(data);
    } else
        return -1;
}

void UIListState::SetCircular(bool c, bool b) {
    mCircular = c;
    if (b) {
        SetSelected(0, -1, true);
    }
}

void UIListState::Poll(float f1) {
    if (IsScrolling()) {
        if (mStepTime == -1) {
            mStepTime = f1;
            mCallback->StartScroll(*this, CurrentScroll() > 0 ? 1 : -1, true);
        }
        if (f1 >= mSpeed + mStepTime) {
            int add = CurrentScroll() > 0 ? 1 : -1;
            mFirstShowing = WrapShowing(mFirstShowing + add);
            mCallback->CompleteScroll(*this);
            if (IsScrolling()) {
                mStepTime = (f1 - (f1 - (mStepTime + mSpeed)));
                mCallback->StartScroll(*this, CurrentScroll() > 0 ? 1 : -1, true);
            } else {
                mStepTime = -1;
            }
        }
        if (mFirstShowing == mTargetShowing || mSpeed == 0) {
            mStepPercent = 0;
            if (mSpeed == 0) {
                while (IsScrolling()) {
                    Poll(f1);
                }
            }
        } else {
            mStepPercent = (f1 - mStepTime) / mSpeed;
        }
    } else {
        mStepTime = -1;
        mStepPercent = 0;
    }
}

bool UIListState::CanScrollBack(bool b) const {
    if (mCircular)
        return true;
    int count = b ? Display2Data(mSelectedDisplay) : Showing2Data(mFirstShowing);
    for (count = count - 1; count >= 0; count--) {
        if (mProvider->IsActive(count))
            return true;
    }
    return false;
}

bool UIListState::CanScrollNext(bool b) const {
    if (mCircular)
        return true;
    else if (b) {
        for (int data = Display2Data(mSelectedDisplay) + 1; data < mProvider->NumData();
             data++) {
            if (mProvider->IsActive(data))
                return true;
        }
    } else {
        return MaxFirstShowing() > mFirstShowing;
    }
    return false;
}

bool UIListState::ShouldHoldDisplayInPlace(int i2) const {
    bool b2 = (mTargetShowing > mFirstShowing && i2 == 0)
        || (mTargetShowing < mFirstShowing && i2 == -1);
    if (!b2) {
        return false;
    }
    return (SnappedDataForDisplay(i2) >= 0)
        && (i2 + 1 != NumDisplay() && Display2Data(i2 + 1) != -1)
        && (!Provider()->IsSnappableAtData(Display2Data(i2 + 1)));
}

int UIListState::State2Data(const ScrollState &state) const {
    int disp;
    if (mCircular) {
        disp = SelectedDisplay();
    } else {
        disp = state.mSelected;
    }
    if (mScrollPastMinDisplay) {
        disp -= mMinDisplay;
    }
    return Showing2Data(state.mTarget + disp);
}

void UIListState::Scroll(int i1, bool b2) {
    if (mFirstShowing == mTargetShowing) {
        ScrollState state;
        bool b4 = BuildScroll(i1, mTargetShowing, mSelectedDisplay, state);
        if (mCircular) {
            while (true) {
                if (b2 || mProvider->IsActive(State2Data(state))) {
                    mTargetShowing = state.mTarget;
                    MILO_ASSERT(state.mSelected == mSelectedDisplay, 0x1D6);
                    return;
                }
                if (mTargetShowing == state.mTarget) {
                    return;
                }
                int old = state.mTarget;
                BuildScroll(i1 > 0 ? 1 : -1, state.mTarget, state.mSelected, state);
                if (old == state.mTarget) {
                    break;
                }
            }
        } else {
            bool b1 = false;
            while (!b2) {
                if (mProvider->IsActive(State2Data(state))) {
                    break;
                }
                if (b1) {
                    return;
                }
                i1 = i1 > 0 ? 1 : -1;
                b4 = BuildScroll(i1, state.mTarget, state.mSelected, state);
                if (i1 == 1) {
                    b1 = state.mTarget == MaxFirstShowing()
                        && state.mSelected == ScrollMaxDisplay();
                } else {
                    bool zeroTarget = state.mTarget == 0;
                    if (mScrollPastMinDisplay) {
                        b1 = zeroTarget && state.mSelected == mMinDisplay;
                    } else {
                        b1 = zeroTarget && state.mSelected == 0;
                    }
                }
            }
            mTargetShowing = state.mTarget;
            mSelectedDisplay = state.mSelected;
            if (!b2 && !b4) {
                mCallback->StartScroll(*this, i1 > 0 ? 1 : -1, false);
                mCallback->CompleteScroll(*this);
            }
        }
    }
}

void UIListState::PageScroll(int i1) {
    int i2 = i1 > 0 ? 1 : -1;
    if (mCircular) {
        i2 *= mNumDisplay;
    } else if (i2 > 0) {
        if (mSelectedDisplay == mNumDisplay - 1 || mSelectedDisplay == mMaxDisplay) {
            i2 = mNumDisplay - mMinDisplay;
        } else {
            i2 = ((mNumDisplay - mMinDisplay) - mSelectedDisplay) - 1;
        }
    } else if (i2 < 0) {
        if (mSelectedDisplay == mMinDisplay) {
            i2 = mMinDisplay - mNumDisplay;
        } else {
            i2 = Min(0, mMinDisplay - mSelectedDisplay);
        }
    }
    Scroll(i2, false);
}

void UIListState::SetSelectedSimulateScroll(int i1) {
    int showing = WrapShowing(i1);
    mFirstShowing = mTargetShowing;
    mStepTime = -1;
    mStepPercent = 0;
    int i5 = showing - SelectedNoWrap();
    if (i5 != 0) {
        if (abs(i5) > mNumDisplay * 2) {
            SetSelected(showing - (i5 > 0 ? 1 : -1) * 2 * mNumDisplay, -1, true);
        }
        while (SelectedNoWrap() != showing) {
            Scroll(showing - SelectedNoWrap() > 0 ? 1 : -1, true);
            mStepTime = -1;
            mStepPercent = 0;
            mFirstShowing = mTargetShowing;
        }
        MILO_ASSERT(showing == SelectedNoWrap(), 0x1BC);
        mCallback->CompleteScroll(*this);
    }
}

int UIListState::MinDisplay() const { return 1; }

int UIListState::MaxDisplay() const { return 1; }

bool UIListState::ScrollPastMinDisplay() const { return false; }

bool UIListState::ScrollPastMaxDisplay() const { return mScrollPastMaxDisplay; }

bool UIListState::IsScrolling() const { return mFirstShowing != mTargetShowing; }

int UIListState::NumShowing() const { return mProvider->NumData() - mHiddenData.size(); }

UIListProvider *UIListState::Provider() { return mProvider; }

UIListProvider *UIListState::Provider() const { return mProvider; }

void UIListState::SetProvider(UIListProvider *provider, RndDir *rdir) {
    MILO_ASSERT(provider, 0x126);
    provider->InitData(rdir);
    mProvider = provider;
    mHiddenData.clear();
    for (int i = 0; i < mProvider->NumData(); i++) {
        if (mProvider->IsHidden(i)) {
            mHiddenData.push_back(i);
        }
    }
    SetSelected(0, -1, true);
}

int UIListState::ScrollToTarget(int i1) const {
    int showing = i1 - mFirstShowing;
    if (mCircular) {
        int u2;
        if (showing > 0) {
            u2 = showing - NumShowing();
        } else {
            u2 = showing + NumShowing();
        }
        int i3 = abs(u2);
        int ivar1 = abs(showing);
        if (i3 < ivar1) {
            return u2;
        }
        if (i3 == ivar1) {
            return 1;
        }
    }
    return showing;
}

bool UIListState::BuildScroll(int i1, int i2, int i3, ScrollState &state) const {
    state.mTarget = i2;
    state.mSelected = i3;
    if (mFirstShowing != i2) {
        int cmp1 = i1 > 0 ? 1 : -1;
        int cmp2 = ScrollToTarget(i2) > 0 ? 1 : -1;
        if (cmp1 != cmp2) {
            return false;
        }
    }
    if (mCircular) {
        int showing = WrapShowing(state.mTarget + i1);
        int scroll = ScrollToTarget(state.mTarget);
        if (scroll) {
            int i7 = scroll > 0 ? 1 : -1;
            int i8 = ScrollToTarget(showing) > 0 ? 1 : -1;
            if (i7 != i8) {
                return false;
            }
        }
        state.mTarget = showing;
    } else {
        state.mSelected += i1;
        int disp = ScrollMaxDisplay();
        if (mScrollPastMinDisplay) {
            disp = Max(mMinDisplay, disp);
        }
        if (state.mSelected < 0) {
            state.mTarget += state.mSelected;
            state.mSelected = Min(i2, mMinDisplay);
        } else if (state.mSelected > disp) {
            state.mTarget += state.mSelected - disp;
            state.mSelected = disp;
        } else {
            if (!mScrollPastMinDisplay || state.mSelected >= mMinDisplay) {
                int tmp2 = state.mTarget;
                if (state.mSelected < mMinDisplay) {
                    state.mTarget = Max(0, tmp2 - 1);
                }
                state.mSelected = (state.mSelected - state.mTarget) + tmp2;
                return state.mTarget != tmp2;
            }
            state.mTarget -= mMinDisplay - state.mSelected;
            state.mSelected = mMinDisplay;
        }
        state.mSelected = Clamp(0, disp, state.mSelected);
        int max = MaxFirstShowing();
        state.mTarget = Clamp(0, max, state.mTarget);
    }
    return state.mSelected == i3 || state.mTarget != i2;
}

float UIListState::StepPercent() const { return mStepPercent; }
