#include "ui/UIButton.h"
#include "obj/Object.h"
#include "ui/UILabel.h"
#include "utl/BinStream.h"

void UIButton::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

void UIButton::PostLoad(BinStream &bs) { UILabel::PostLoad(bs); }

BEGIN_COPYS(UIButton)
    CREATE_COPY_AS(UIButton, f);
    MILO_ASSERT(f, 0x25);
    COPY_SUPERCLASS(UILabel)
END_COPYS

void UIButton::Save(BinStream &bs) {
    bs << 0;
    SAVE_SUPERCLASS(UILabel)
}

void UIButton::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    UILabel::PreLoad(bs);
}

BEGIN_PROPSYNCS(UIButton)
    SYNC_SUPERCLASS(UILabel)
END_PROPSYNCS

DataNode UIButton::OnMsg(const ButtonDownMsg &msg) {
    if (msg.GetAction() == kAction_Confirm && GetState() == UIComponent::kFocused) {
        SendSelect(msg.GetUser());
        return 1;
    }
    return DATA_UNHANDLED;
}

BEGIN_HANDLERS(UIButton)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(UILabel)
END_HANDLERS

UIButton::UIButton() {}

void UIButton::Init() { REGISTER_OBJ_FACTORY(UIButton) }
