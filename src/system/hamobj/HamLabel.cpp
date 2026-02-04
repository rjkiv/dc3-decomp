#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UITransitionHandler.h"
#include "utl/BinStream.h"
#include "utl/Locale.h"
#include "utl/Str.h"

HamLabel::HamLabel()
    : UITransitionHandler(this), unk178(""), unk180(0), mCanHaveFocus(0) {}

HamLabel::~HamLabel() {}

BEGIN_HANDLERS(HamLabel)
    HANDLE_ACTION(
        start_count, Count(_msg->Int(2), _msg->Int(3), _msg->Float(4), _msg->Sym(5))
    )
    HANDLE_ACTION(finish_count, FinishCount())
    HANDLE_ACTION(set_move_name, SetMoveName(_msg->Obj<HamMove>(2)))
    HANDLE_SUPERCLASS(UILabel)
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

void HamLabel::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

INIT_REVS(17, 0)

void HamLabel::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(17, 0)
    if (d.rev >= 17) {
        UILabel::PreLoad(d.stream);
    } else {
        MILO_FAIL("Can't load HamLabel older than rev %d", 17);
    }
    d.PushRev(this);
}

void HamLabel::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    UILabel::PostLoad(d.stream);
    LoadHandlerData(d.stream);
}

void HamLabel::Count(int i1, int i2, float f3, Symbol s) {
    unk168.clear();
    unk168.push_back(Key<float>(TheTaskMgr.UISeconds() * 1000, i1));
    unk168.push_back(Key<float>((float)(i2) + f3, i2));
    unk174 = s;
}

void HamLabel::FinishCount() {
    if (unk168.size() >= 2) {
        SetTokenFmt(unk174, LocalizeSeparatedInt(unk168[1].value, TheLocale));
        unk168.clear();
    }
}

void HamLabel::SetMoveName(HamMove *move) {
    String str;
    if (move) {
        str = move->DisplayName();
        mMarkup = str.find('<') != FixedString::npos;
    }
    SetDisplayText(str.c_str(), true);
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

void HamLabel::SetDisplayText(const char *cc, bool b2) {
    if (!streq(cc, unk178.c_str()) || b2 != unk180) {
        unk178 = cc;
        unk180 = b2;
        UITransitionHandler::StartValueChange();
    }
}

void HamLabel::FinishValueChange() {
    UILabel::SetDisplayText(unk178.c_str(), unk180);
    UITransitionHandler::FinishValueChange();
}

void HamLabel::Init() { REGISTER_OBJ_FACTORY(HamLabel); }
