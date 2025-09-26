#include "hamobj/HamLabel.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UITransitionHandler.h"
#include "utl/Locale.h"

HamLabel::HamLabel()
    : UITransitionHandler(this), unk178(""), unk180(0), mCanHaveFocus(0) {}
HamLabel::~HamLabel() {}

BEGIN_HANDLERS(HamLabel)
    HANDLE_ACTION(
        start_count, Count(_msg->Int(2), _msg->Int(3), _msg->Float(4), _msg->Sym(5))
    )
    HANDLE_ACTION(finish_count, FinishCount())
END_HANDLERS

BEGIN_PROPSYNCS(HamLabel)
    SYNC_PROP_SET(in_anim, GetInAnim(), SetInAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_SET(out_anim, GetOutAnim(), SetOutAnim(_val.Obj<RndAnimatable>()))
    SYNC_SUPERCLASS(UILabel)
END_PROPSYNCS

BEGIN_SAVES(HamLabel)
    SAVE_REVS(17, 0)
    SAVE_SUPERCLASS(UILabel)
    SaveHandlerData(bs);
END_SAVES

BEGIN_COPYS(HamLabel)
    COPY_SUPERCLASS(UILabel)
    CREATE_COPY(HamLabel)
    BEGIN_COPYING_MEMBERS
        CopyHandlerData(c);
    END_COPYING_MEMBERS
END_COPYS

void HamLabel::Init() { REGISTER_OBJ_FACTORY(HamLabel); }

void HamLabel::FinishCount() {
    if (unk168.size() >= 2) {
        SetTokenFmt(unk174, LocalizeSeparatedInt(unk168[1].value, TheLocale));
        unk168.clear();
    }
}

void HamLabel::Poll() {
    UILabel::Poll();
    if (unk168.size() >= 2) {
        float f3 = 0;
        float ui_ms = TheTaskMgr.UISeconds() * 1000;
        unk168.AtFrame(ui_ms, f3);
        SetTokenFmt(unk174, LocalizeSeparatedInt(0, TheLocale));
        if (f3 < ui_ms) {
            unk168.clear();
            TheUI->Handle(HamLabelCountDoneMsg(this), false);
        }
    }
    UpdateHandler();
}

void HamLabel::Count(int i1, int i2, float f3, Symbol s) {
    unk168.clear();
    float ui_ms = TheTaskMgr.UISeconds() * 1000;
    unk168.push_back(Key<float>(ui_ms, i1));
    unk168.push_back(Key<float>((float)(i2) + f3, i2));
    unk174 = s;
}

void HamLabel::FinishValueChange() {
    UILabel::SetDisplayText(unk178.c_str(), unk180);
    UITransitionHandler::FinishValueChange();
}

void HamLabel::SetDisplayText(const char *cc, bool b2) {
    if (!streq(cc, unk178.c_str()) || b2 != unk180) {
        unk178 = cc;
        unk180 = b2;
        UITransitionHandler::StartValueChange();
    }
}
