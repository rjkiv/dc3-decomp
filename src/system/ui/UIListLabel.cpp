#include "ui/UIListLabel.h"
#include "math/Geo.h"
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

INIT_REVS(1, 1)

BEGIN_LOADS(UIListLabel)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 1)
    LOAD_SUPERCLASS(UIListSlot)
    d >> mLabel;
    if (d.rev < 1) {
        String str;
        d >> str;
    }
    if (d.altRev >= 1) {
        d >> unk8c;
    }
END_LOADS

UIListSlotElement *UIListLabel::CreateElement(UIList *uilist) {
    MILO_ASSERT(mLabel, 0x86);
    Hmx::Object *obj = Hmx::Object::NewObject(mLabel->ClassName());
    UILabel *l = dynamic_cast<UILabel *>(obj);
    MILO_ASSERT(l, 0x89);
    l->Copy(mLabel, kCopyShallow);
    l->SetTextToken(gNullStr);
    return new UIListLabelElement(this, l);
}

const char *UIListLabel::GetDefaultText() const {
    if (mLabel)
        return mLabel->GetDefaultText();
    return gNullStr;
}

UILabel *UIListLabel::ElementLabel(int display) const {
    if (mElements.size() == 0)
        return nullptr;
    else {
        MILO_ASSERT_RANGE(display, 0, mElements.size(), 0x74);
        UIListLabelElement *le = dynamic_cast<UIListLabelElement *>(mElements[display]);
        MILO_ASSERT(le, 0x77);
        return le->Label();
    }
}

#pragma endregion UIListLabel
#pragma region UIListLabelElement

UIListLabelElement::~UIListLabelElement() { delete mLabel; }

void UIListLabelElement::Draw(const Transform &tf, float f3, UIColor *col, Box *box) {
    mLabel->SetWorldXfm(tf);
    if (box) {
        Box localBox;
        localBox.Set(box->mMin, box->mMax);
        const Hmx::Rect &r = mLabel->DrawRect();
        Vector3 v90;
        v90.Set(r.x, 0, r.y);
        localBox.GrowToContain(v90, false);
        Vector3 v80;
        v80.Set(r.w + r.x, 0, r.h + r.y);
        localBox.GrowToContain(v80, false);
        box->GrowToContain(localBox.mMin, false);
        box->GrowToContain(localBox.mMax, false);
    } else {
        float *alphas = (float *)_alloca(mLabel->NumStyles() * sizeof(float));

        for (int i = 0; i < mLabel->Styles().size(); i++) {
            RndText::Style &curStyle = mLabel->Style(i);
            alphas[i] = curStyle.mInfo.mFontColor.alpha;
        }
        mLabel->LStyle(0).mColorOverride = col;
        if (mListLabel->GetUnk8c()) {
            for (int i = 1; i < mLabel->Styles().size(); i++) {
                mLabel->LStyle(i).mColorOverride = col;
            }
        }
        for (int i = 0; i < mLabel->Styles().size(); i++) {
            mLabel->Style(i).mInfo.mFontColor.alpha *= f3;
        }
        mLabel->DrawShowing();
        for (int i = 0; i < mLabel->Styles().size(); i++) {
            mLabel->Style(i).mInfo.mFontColor.alpha = alphas[i];
        }
    }
}

#pragma endregion UIListLabelElement
