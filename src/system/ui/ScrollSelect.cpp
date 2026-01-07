#include "ui/ScrollSelect.h"
#include "UIComponent.h"
#include "obj/Object.h"
#include "os/Joypad.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

void ScrollSelect::Store() { mSelectedAux = SelectedAux(); }
void ScrollSelect::Reset() { mSelectedAux = -1; }
bool ScrollSelect::CanScroll() const { return !mSelectToScroll || mSelectedAux != -1; }

ScrollSelect::ScrollSelect() : mSelectToScroll(0) { Reset(); }

BEGIN_PROPSYNCS(ScrollSelect)
    SYNC_PROP(select_to_scroll, mSelectToScroll)
END_PROPSYNCS

DataNode ScrollSelect::SendScrollSelected(UIComponent *comp, LocalUser *user) {
    static UIComponentScrollSelectMsg scroll_select_msg(0, 0, 0);
    scroll_select_msg[0] = comp;
    scroll_select_msg[1] = user;
    scroll_select_msg[2] = mSelectedAux != -1;
    return TheUI->Handle(scroll_select_msg, false);
    return 0;
}

UIComponent::State ScrollSelect::DrawState(UIComponent *comp) const {
    static Symbol selected("selected");
    bool ret = false;
    if (!mSelectToScroll || mSelectedAux == -1)
        return comp->GetState();
    return UIComponent::kSelected;
}

bool ScrollSelect::CatchNavAction(JoypadAction act) const { return false; }

bool ScrollSelect::SelectScrollSelect(UIComponent *comp, LocalUser *user) {
    if (mSelectToScroll) {
        if (mSelectedAux == -1)
            Store();
        else
            Reset();
        SendScrollSelected(comp, user);
        return true;
    } else
        return false;
}

bool ScrollSelect::RevertScrollSelect(
    UIComponent *comp, LocalUser *user, Hmx::Object *obj
) {
    int oldAux = mSelectedAux;
    if (oldAux != -1) {
        int selAux = SelectedAux();
        bool somenum = oldAux != selAux;
        SetSelectedAux(oldAux);
        mSelectedAux = -1;
        DataNode node(kDataUnhandled, 0);
        if (somenum && obj) {
            // node = obj->Handle(UIComponentScrollMsg(comp, user), false);
        }
        if (node.Type() == kDataUnhandled) {
            node = SendScrollSelected(comp, user);
        }
        if (somenum) {
            if (node.Type() == kDataUnhandled) {
                // TheUI->Handle(UIComponentScrollMsg(comp, user), false);
            }
        }
        return true;
    } else
        return false;
}

DataNode ScrollSelect::Handle(DataArray *_msg, bool _warn) {
    Symbol sym = _msg->Sym(1);

    MessageTimer timer(
        (MessageTimer::Active()) ? dynamic_cast<Hmx::Object *>(this) : 0, sym
    );

    HANDLE_EXPR(is_scroll_selected, IsScrollSelected())
    HANDLE_ACTION(reset, Reset())
    if (_warn)
        MILO_NOTIFY(
            "%s(%d): %s unhandled msg: %s",
            __FILE__,
            0x58,
            PathName(dynamic_cast<Hmx::Object *>(this)),
            sym
        );
    return DATA_UNHANDLED;
}
