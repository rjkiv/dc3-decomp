#include "ui/UIListLabel.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListSlot.h"
#include "utl/Symbol.h"

#pragma region UIListLabel

UIListLabel::UIListLabel() : mLabel(this), unk8c(0) {}

BEGIN_PROPSYNCS(UIListLabel)
    SYNC_PROP(label, mLabel)
    SYNC_PROP(highlight_alt_styles, unk8c)
    SYNC_SUPERCLASS(UIListSlot)
END_PROPSYNCS

BEGIN_SAVES(UIListLabel)
    SAVE_REVS(1, 1)
    SAVE_SUPERCLASS(UIListSlot)
    bs << mLabel;
    bs << unk8c;
END_SAVES

BEGIN_COPYS(UIListLabel)
    COPY_SUPERCLASS(UIListSlot)
    CREATE_COPY_AS(UIListLabel, l)
    MILO_ASSERT(l, 0xba);
    COPY_MEMBER_FROM(l, mLabel)
    COPY_MEMBER_FROM(l, unk8c)
END_COPYS

BEGIN_LOADS(UIListLabel)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(UIListSlot)
    bs >> mLabel;
END_LOADS

const char *UIListLabel::GetDefaultText() const {
    if (mLabel)
        return mLabel->GetDefaultText();
    return gNullStr;
}

UILabel *UIListLabel::ElementLabel(int display) const {
    if (mElements.empty())
        return 0;
    else {
        MILO_ASSERT((0) <= (display) && (display) < (mElements.size()), 0x74);
        UIListLabelElement *le = dynamic_cast<UIListLabelElement *>(mElements[display]);
        MILO_ASSERT(le, 0x77);
        return le->mLabel;
    }
    return 0;
}

UIListSlotElement *UIListLabel::CreateElement(UIList *uilist) {
    MILO_ASSERT(mLabel, 0x86);
    UILabel *l = dynamic_cast<UILabel *>(Hmx::Object::NewObject(mLabel->ClassName()));
    MILO_ASSERT(l, 0x89);
    // l->ResourceCopy(mLabel);
    l->SetTextToken(gNullStr);
    return nullptr;
}

#pragma endregion UIListLabel
#pragma region UIListLabelElement

UIListLabelElement::~UIListLabelElement() { delete mLabel; }

void UIListLabelElement::Draw(const Transform &tf, float f, UIColor *col, Box *box) {}

#pragma endregion UIListLabelElement
