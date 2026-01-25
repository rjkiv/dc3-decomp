#include "ui/UILabel.h"

#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Text.h"
#include "rndobj/Trans.h"
#include "ui/UI.h"
#include "ui/UILabelDir.h"
#include "ui/UIListWidget.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Locale.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <cstring>

bool UILabel::sDeferUpdate;

void UILabel::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

UILabel::UILabel() : unk122(1), unk124(this) {
    unk124.resize(1);
    unk120 = 0;
    unk121 = false;
}

BEGIN_PROPSYNCS(UILabel)
    SYNC_PROP_SET(text_token, mTextToken, SetTextToken(_val.ForceSym()))
    SYNC_PROP_SET(icon, unk118, SetIcon(_val.Str(0)[0]))
    SYNC_PROP(edit_text, unk118)
    SYNC_PROP(draw_width, unkb4)
    SYNC_PROP(styles, unk124)
    SYNC_SUPERCLASS(RndText)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_COPYS(UILabel)
    COPY_SUPERCLASS(UIComponent)
    COPY_SUPERCLASS(RndText)
    CREATE_COPY(UILabel)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTextToken)
        COPY_MEMBER(unk118)
        //looks like an strcpy here
    END_COPYING_MEMBERS
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
END_COPYS

BEGIN_SAVES(UILabel)
END_SAVES

void UILabel::PreLoad(BinStream &) {}

void UILabel::PostLoad(BinStream &bs) {}

Symbol UILabel::TextToken() { return mTextToken; }

void UILabel::Poll() {}

void UILabel::Highlight() {}

void UILabel::DrawShowing() {}

void UILabel::SetTextToken(Symbol s) {
    mTextToken = s;

    SetTokenFmtImp(mTextToken, 0, 0, 0, true);
}

void UILabel::SetInt(int i, bool b) {
    if (b) {
        SetDisplayText(LocalizeSeparatedInt(i, TheLocale), true);
    } else
        SetDisplayText(MakeString("%d", i), true);
}

void UILabel::SetFloat(const char *cc, float f) {
    SetDisplayText(LocalizeFloat(cc, f), true);
}

void UILabel::SetDateTime(DateTime const &dt, Symbol s) {
    String str(Localize(s, false, TheLocale));
    dt.Format(str);
    SetDisplayText(str.c_str(), true);
}

void UILabel::SetIcon(char c) {
    unk120 = c;
    if (c == '\0' && TheLoadMgr.EditMode() != 0) {
        SetEditText(unk118.c_str());
        return;
    }
}

void UILabel::SetTokenFmt(const DataArray *) {}

RndText::Style &UILabel::Style(int) { return Style(0); }

void UILabel::SetPrelocalizedString(String &s) {}

void UILabel::SetSubtitle(const DataArray *) {}

void UILabel::SetTimeHMS(int, bool) {}

bool UILabel::CheckValid(bool) { return false; }

void UILabel::SetEditText(const char *c) {}

char const *UILabel::GetDefaultText() const {
    if (unk120 != 0) {
        return &unk120;
    }

    if (TheLoadMgr.EditMode() && !unk118.empty())
        return unk118.c_str();
    else
        return Localize(mTextToken, nullptr, TheLocale);
}

void UILabel::CenterWithLabel(UILabel *, bool, float) {}

// UILabel::LabelStyle &UILabel::LStyle(int) { return new LabelStyle(0); }

void UILabel::OldResourcePreload(BinStream &) {}

void UILabel::SetDisplayText(const char *cc, bool b) {
    if (b)
        mTextToken = gNullStr;
    RndText::SetText(cc);
    if (strchr(cc, 60)) {
        mMarkup = true;
    }
    if (!sDeferUpdate)
        LabelUpdate(false);
}

void UILabel::Init() {
    REGISTER_OBJ_FACTORY(UILabel);
    UILabelDir::Init();
}

void UILabel::SetTokenFmtImp(
    Symbol s, const DataArray *da1, const DataArray *da2, int i, bool b
) {}

DataNode UILabel::OnSetPrelocalizedString(DataArray const *da) {
    return NULL_OBJ;
}

DataNode UILabel::OnSetTokenFmt(DataArray const *da) { return NULL_OBJ; }

DataNode UILabel::OnSetInt(DataArray const *da) { return DataNode(1); }

DataNode UILabel::OnSetTimeHMS(DataArray const *) { return NULL_OBJ; }

bool UILabel::AllowEditText() const { return false; }

void UILabel::LabelUpdate(bool b) { unk122 = false; }

DataNode UILabel::OnSetHeightFromText(DataArray *) { return NULL_OBJ; }

void UILabel::SetFontMat(char const *c, int i) {
    RndMat *rndmat = nullptr;
    auto labelStyle = LStyle(i);

}

char const *UILabel::GetFontMat(int) { return 0; }

void UILabel::RefreshFontMat(int i) {
    auto mat = GetFontMat(i);
    SetFontMat(mat, i);
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
}

BEGIN_HANDLERS(UILabel)
    HANDLE_ACTION(set_token_fmt, OnSetTokenFmt(_msg))
    HANDLE_ACTION(set_prelocalized_string, OnSetPrelocalizedString(_msg))
    HANDLE_ACTION(set_int, OnSetInt(_msg))
    HANDLE_ACTION(set_float, SetFloat(_msg->Str(2), _msg->Float(3)))
    HANDLE_ACTION(set_time_hms, OnSetTimeHMS(_msg))
    HANDLE_ACTION(center_with_label, CenterWithLabel(_msg->Obj<UILabel>(2), _msg->Int(3), _msg->Float(4)))
    HANDLE_EXPR(get_font_mats, UILabelDir::GetMatVariations(LStyle(_msg->Int(2)).unk14))
    HANDLE_ACTION(set_height_from_text, OnSetHeightFromText(_msg))
    HANDLE_EXPR(draw_rect_width, unkbc)
    HANDLE_ACTION(reload_string, UIComponent::Poll())
    HANDLE_SUPERCLASS(UIComponent)
    HANDLE_SUPERCLASS(RndText)
END_HANDLERS

bool PropSync(UILabel::LabelStyle &, DataNode &, DataArray *, int, PropOp) {
    return false;
}
