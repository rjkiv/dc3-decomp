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
    if (mCircular)
        return mMinDisplay;
    return mSelectedDisplay;
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

void UIListState::SetSelected(int i, int j, bool b) { mTargetShowing = 1.0f; }

void UIListState::SetSpeed(float speed) {
    MILO_ASSERT(speed >= 0, 0x15f);
    mSpeed = speed;
}

float UIListState::Speed() const { return mSpeed; }

void UIListState::SetMinDisplay(int min) {
    MILO_ASSERT(min >= 0, 0x149);
    mMinDisplay = min;

    int x = mSelectedDisplay;
    if (min >= mSelectedDisplay)
        x = min;
    mSelectedDisplay = x;
}

void UIListState::SetMaxDisplay(int max) {
    MILO_ASSERT(max >= -1, 0x150);
    if (TheLoadMgr.EditMode() && (max > mNumDisplay - 1) && max < -1) {
        max = -1;
    }
    mMaxDisplay = max;
}

void UIListState::SetScrollPastMinDisplay(bool b) {
    mScrollPastMinDisplay = b;
    if (!b)
        return;

    int x = mMinDisplay;
    if (mSelectedDisplay >= mMinDisplay)
        x = mSelectedDisplay;
    mSelectedDisplay = x;
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

int UIListState::SnappedDataForDisplay(int i2) const { return -1; }

void UIListState::SetCircular(bool c, bool b) {
    mCircular = c;
    if (b) {
        SetSelected(0, -1, true);
    }
}

void UIListState::Poll(float) {}

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

bool UIListState::ShouldHoldDisplayInPlace(int i2) const { return false; }

void UIListState::Scroll(int, bool) {}

void UIListState::PageScroll(int) {}

void UIListState::SetSelectedSimulateScroll(int) {}

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

int UIListState::ScrollToTarget(int) const { return 1; }
