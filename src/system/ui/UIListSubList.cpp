#include "ui/UIListSubList.h"
#include "obj/Object.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListSlot.h"
#include "ui/UIListWidget.h"
#include "utl/Loader.h"

#pragma region UIListSubList

UIListSubList::UIListSubList() : mList(this) {}

BEGIN_HANDLERS(UIListSubList)
    HANDLE_SUPERCLASS(UIListSlot)
END_HANDLERS

BEGIN_PROPSYNCS(UIListSubList)
    SYNC_PROP(list, mList)
    SYNC_SUPERCLASS(UIListSlot)
END_PROPSYNCS

BEGIN_SAVES(UIListSubList)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIListSlot)
    bs << mList;
END_SAVES

BEGIN_COPYS(UIListSubList)
    COPY_SUPERCLASS(UIListSlot)
    CREATE_COPY_AS(UIListSubList, sl)
    MILO_ASSERT(sl, 0xc0);
    COPY_MEMBER_FROM(sl, mList)
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(UIListSubList)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListSlot)
    d >> mList;
END_LOADS

UIList *UIListSubList::SubList(int index) {
    auto element = mElements[index];
    UIListSubListElement *sle = dynamic_cast<UIListSubListElement *>(element);
    MILO_ASSERT(sle, 0x62);
    return sle->List();
}

void UIListSubList::Draw(
    const UIListWidgetDrawState &drawstate,
    const UIListState &liststate,
    const Transform &tf,
    UIComponent::State compstate,
    Box *box,
    DrawCommand cmd
) {
    if (RootTrans()) {
        int numElements = drawstate.mElements.size();
        for (int i = 0; i < numElements; i++) {
            const UIListElementDrawState &cur = drawstate.mElements[i];
            UIList *uilist = SubList(i);
            switch (cur.mComponentState) {
            case UIComponent::kNormal:
                uilist->SetState(UIComponent::kNormal);
                break;
            case UIComponent::kFocused:
                if (compstate == UIComponent::kFocused) {
                    uilist->SetState(UIComponent::kFocused);
                } else {
                    uilist->SetState(UIComponent::kNormal);
                }
                break;
            case UIComponent::kDisabled:
                uilist->SetState(UIComponent::kDisabled);
                break;
            }
        }
    }
    UIListSlot::Draw(drawstate, liststate, tf, compstate, box, cmd);
}

UIListSlotElement *UIListSubList::CreateElement(UIList *parent) {
    MILO_ASSERT(mList, 0x8d);
    auto obj = Hmx::Object::NewObject(mList->ClassName());
    UIList *l = dynamic_cast<UIList *>(obj);
    MILO_ASSERT(l, 0x90);
    l->SetParent(parent);
    l->SetType(mList->Type());
    l->Copy(mList, kCopyDeep);

    if (parent) {
        l->SetInAnim(parent->GetInAnim());
        l->SetOutAnim(parent->GetOutAnim());
    }
    return new UIListSubListElement(this, l);
}

#pragma endregion UIListSlotElement
#pragma region UIListSubListElement

UIListSubListElement::~UIListSubListElement() { delete mList; }

void UIListSubListElement::Fill(const UIListProvider &prov, int i, int j) {
    UIListProvider *theProvider;
    if (TheLoadMgr.EditMode())
        theProvider = mList;
    else
        theProvider = prov.Provider(i, j, mSlot);
    if (theProvider) {
        mList->SetProvider(theProvider);
        if (0 <= UIListSubList::sNextFillSelection) {
            mList->SetSelected(
                Clamp(0, theProvider->NumData() - 1, UIListSubList::sNextFillSelection),
                -1
            );
            UIListSubList::sNextFillSelection = -1;
        }
    }
}

void UIListSubListElement::Draw(const Transform &tf, float f, UIColor *col, Box *box) {
    mList->SetWorldXfm(tf);
    if (box) {
        Box localbox;
        mList->CalcBoundingBox(localbox);
        box->GrowToContain(localbox.mMin, false);
        box->GrowToContain(localbox.mMax, false);
    } else
        mList->DrawShowing();
}

#pragma endregion UIListSubListElement
