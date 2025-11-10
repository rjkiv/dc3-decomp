#include "ui/UIListSubList.h"
#include "obj/Object.h"
#include "ui/UIList.h"
#include "ui/UIListSlot.h"
#include "utl/Loader.h"

#pragma region UIListSubList

UIListSubList::UIListSubList() : mList(this) {}

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

BEGIN_LOADS(UIListSubList)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListSlot)
    bs >> mList;
END_LOADS

UIList *UIListSubList::SubList(int index) {
    UIListSubListElement *sle = dynamic_cast<UIListSubListElement *>(mElements[index]);
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
    UIListSlot::Draw(drawstate, liststate, tf, compstate, box, cmd);
}

UIListSlotElement *UIListSubList::CreateElement(UIList *parent) {
    MILO_ASSERT(mList, 0x8d);
    UIList *l = dynamic_cast<UIList *>(Hmx::Object::NewObject(mList->ClassName()));
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
