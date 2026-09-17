#include "ui/UIListSlot.h"
#include "macros.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListProvider.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "utl/Std.h"

UIListSlot::UIListSlot() : mSlotDrawType(kUIListSlotDrawAlways), mNextElement(0) {}

BEGIN_PROPSYNCS(UIListSlot)
    SYNC_PROP_SET(
        slot_draw_type, (int)mSlotDrawType, mSlotDrawType = (UIListSlotDrawType)_val.Int()
    )
    SYNC_SUPERCLASS(UIListWidget)
END_PROPSYNCS

BEGIN_SAVES(UIListSlot)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIListWidget)
    bs << mSlotDrawType;
END_SAVES

BEGIN_COPYS(UIListSlot)
    COPY_SUPERCLASS(UIListWidget)
    CREATE_COPY_AS(UIListSlot, s)
    MILO_ASSERT(s, 0xe1);
    COPY_MEMBER_FROM(s, mSlotDrawType)
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(UIListSlot)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListWidget)
    int ty;
    bs >> ty;
    mSlotDrawType = (UIListSlotDrawType)ty;
END_LOADS

void UIListSlot::ResourceCopy(const UIListWidget *w) {
    UIListWidget::ResourceCopy(w);
    mMatchName = w->Name();
}

void UIListSlot::CreateElements(UIList *uilist, int count) {
    if (RootTrans()) {
        ClearElements();
        for (int i = 0; i < count; i++) {
            mElements.push_back(CreateElement(uilist));
        }
        mNextElement = CreateElement(uilist);
    }
}

void UIListSlot::Draw(
    const UIListWidgetDrawState &drawState,
    const UIListState &listState,
    const Transform &xfm,
    UIComponent::State compState,
    Box *box,
    DrawCommand cmd
) {
    RndTransformable *root = RootTrans();
    if (root) {
        int numDrawElements = drawState.mElements.size();
        if (numDrawElements > mElements.size()) {
            MILO_FAIL(
                "%i isn't enough elements (need %i)", mElements.size(), numDrawElements
            );
        }
        Transform tfc0 = root->WorldXfm();
        UIListProvider *prov = listState.Provider();
        for (int i = 0; i < numDrawElements; i++) {
            const UIListElementDrawState &curElDrawState = drawState.mElements[i];
            if (curElDrawState.mDraw) {
                UIColor *c = nullptr;
                float alpha = 1;
                if (!box) {
                    if (((mSlotDrawType == kUIListSlotDrawHighlight
                          || mSlotDrawType == kUIListSlotDrawHighlightFullAlpha)
                         && (curElDrawState.mDisplay != drawState.mHighlightDisplay))
                        || ((mSlotDrawType == kUIListSlotDrawNoHighlight
                             || mSlotDrawType == kUIListSlotDrawNoHighlightFullAlpha)
                            && (curElDrawState.mDisplay == drawState.mHighlightDisplay))) {
                        continue;
                    }
                    UIListWidgetState element_state = prov->SlotElementStateOverride(
                        curElDrawState.mShowing,
                        curElDrawState.mData,
                        this,
                        curElDrawState.mElementState
                    );
                    UIComponent::State cs = curElDrawState.mComponentState;
                    UIColor *color = DisplayColor(element_state, cs);
                    c = prov->SlotColorOverride(
                        curElDrawState.mShowing, curElDrawState.mData, this, color
                    );
                    if (mSlotDrawType != kUIListSlotDrawAlwaysFullAlpha
                        && mSlotDrawType != kUIListSlotDrawHighlightFullAlpha
                        && mSlotDrawType != kUIListSlotDrawNoHighlightFullAlpha) {
                        alpha = curElDrawState.mAlpha;
                    } else {
                        alpha = 1;
                    }
                    if (cs == UIComponent::kDisabled) {
                        alpha *= DisabledAlphaScale();
                    }
                    prov->PreDraw(curElDrawState.mShowing, curElDrawState.mData, this);
                }
                Transform tf100 = tfc0;
                if (ParentList()) {
                    ParentList()->AdjustTrans(tf100, curElDrawState);
                }
                CalcXfm(xfm, curElDrawState.mPos, tf100);
                ScaleDiagonal(curElDrawState.unk14, tf100.m);
                if (cmd != kExcludeFirst || i > 0) {
                    mElements[i]->Draw(tf100, alpha, c, box);
                    if (cmd == kDrawFirst) {
                        return;
                    }
                }
            }
        }
    }
}

void UIListSlot::Fill(const UIListProvider &prov, int display, int j, int k) {
    if (RootTrans()) {
        MILO_ASSERT(display < mElements.size(), 0x98);
        mElements[display]->Fill(prov, j, k);
    }
}

void UIListSlot::StartScroll(int i, bool b) {
    if (b && RootTrans()) {
        mElements.insert(i < 0 ? mElements.begin() : mElements.end(), mNextElement);
        mNextElement = 0;
    }
}

void UIListSlot::CompleteScroll(const UIListState &state, int i2) {
    if (RootTrans()) {
        if (mElements.size() == state.NumDisplay() + 1) {
            UIListSlotElement *element = mElements[i2 > 0 ? 0 : state.NumDisplay()];
            mElements.erase(std::find(mElements.begin(), mElements.end(), element));
            mNextElement = element;
        }
    }
}

void UIListSlot::Poll() {
    FOREACH (it, mElements) {
        (*it)->Poll();
    }
}

bool UIListSlot::Matches(const char *cc) const { return streq(mMatchName.c_str(), cc); }

const char *UIListSlot::MatchName() const { return mMatchName.c_str(); }

void UIListSlot::ClearElements() {
    DeleteAll(mElements);
    RELEASE(mNextElement);
}
