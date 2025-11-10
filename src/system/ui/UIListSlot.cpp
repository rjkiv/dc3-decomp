#include "ui/UIListSlot.h"
#include "macros.h"
#include "obj/Object.h"
#include "ui/UIList.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "utl/Std.h"

UIListSlot::UIListSlot() : mSlotDrawType(kUIListSlotDrawAlways), mNextElement(0) {}

BEGIN_PROPSYNCS(UIListSlot)
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
    const UIListWidgetDrawState &,
    const UIListState &,
    const Transform &,
    UIComponent::State,
    Box *,
    DrawCommand
) {}

void UIListSlot::Fill(const UIListProvider &prov, int display, int j, int k) {
    if (RootTrans()) {
        MILO_ASSERT(display < mElements.size(), 0x98);
        mElements[display]->Fill(prov, j, k);
    }
}

void UIListSlot::StartScroll(int i, bool b) {}

void UIListSlot::CompleteScroll(const UIListState &, int) {}

void UIListSlot::Poll() {
    FOREACH (it, mElements) {
        (*it)->Poll();
    }
}

bool UIListSlot::Matches(const char *cc) const {
    return strcmp(mMatchName.c_str(), cc) == 0;
}

const char *UIListSlot::MatchName() const { return mMatchName.c_str(); }

void UIListSlot::ClearElements() {
    DeleteAll(mElements);
    RELEASE(mNextElement);
}
