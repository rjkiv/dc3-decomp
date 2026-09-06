#include "ui/UILabel.h"

#include "macros.h"
#include "math/Color.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "obj/PropSync_p.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/FontBase.h"
#include "rndobj/Text.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UI.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UILabelDir.h"
#include "ui/UIListWidget.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Locale.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include <cmath>
#include <cstring>

bool UILabel::sDeferUpdate = false;
UILabel *gMe = nullptr;

float GetTextSizeFromPctHeight(float);
float GetPctHeightFromTextSize(float);

UILabel::UILabel() : unk122(1), mLabelStyles(this) {
    mLabelStyles.resize(1);
    unk120 = 0;
    unk121 = false;
}

BEGIN_HANDLERS(UILabel)
    HANDLE(set_token_fmt, OnSetTokenFmt)
    HANDLE(set_prelocalized_string, OnSetPrelocalizedString)
    HANDLE(set_int, OnSetInt)
    HANDLE_ACTION(set_float, SetFloat(_msg->Str(2), _msg->Float(3)))
    HANDLE(set_time_hms, OnSetTimeHMS)
    HANDLE_ACTION(
        center_with_label,
        CenterWithLabel(_msg->Obj<UILabel>(2), _msg->Int(3), _msg->Float(4))
    )
    HANDLE_EXPR(
        get_font_mats, UILabelDir::GetMatVariations(LStyle(_msg->Int(2)).mFontResource)
    )
    HANDLE(set_height_from_text, OnSetHeightFromText)
    HANDLE_EXPR(draw_rect_width, mDrawRect.w)
    HANDLE_ACTION(reload_string, (SetTextToken(mTextToken), unk122 = true))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(UILabel::LabelStyle)
    int idx = (&o - &gMe->LStyle(0));
    SYNC_PROP_MODIFY(font_resource, o.mFontResource, gMe->RefreshFontMat(idx))
    SYNC_PROP(color_override, o.mColorOverride)
    SYNC_PROP_SET(
        font_mat_variation, gMe->GetFontMat(idx), gMe->SetFontMat(_val.Str(), idx);
        if (!UILabel::sDeferUpdate) { gMe->LabelUpdate(false); }
    )
    RndText::Style &textStyle = gMe->Style(idx);
    SYNC_PROP_SET(
        text_size,
        GetPctHeightFromTextSize(textStyle.mInfo.mSize),
        textStyle.mInfo.mSize = GetTextSizeFromPctHeight(_val.Float());
        if (!UILabel::sDeferUpdate) { gMe->LabelUpdate(false); }
    )
    SYNC_PROP_SET(
        font_alpha,
        textStyle.mInfo.mFontColor.alpha,
        textStyle.mInfo.mFontColor.alpha = _val.Float()
    )
    SYNC_PROP_MODIFY(
        italics, textStyle.mInfo.mItalics, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        kerning, textStyle.mInfo.mKerning, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        z_offset, textStyle.mInfo.mZOffset, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
    SYNC_PROP_MODIFY(
        blacklight, textStyle.mBlacklight, if (!UILabel::sDeferUpdate) {
            gMe->LabelUpdate(false);
        }
    )
END_CUSTOM_PROPSYNC

bool PropSync(
    ObjVector<UILabel::LabelStyle> &v, DataNode &val, DataArray *prop, int i, PropOp op
) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize, 0x4A9);
        val = (int)v.size();
        return true;
    } else {
        int idx = prop->Int(i++);
        MILO_ASSERT(v.size() == gMe->Styles().size(), 0x4B0);
        auto labelIt = v.begin() + idx;
        auto stylesIt = gMe->Styles().begin() + idx;
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            return PropSync(*labelIt, val, prop, i, op);
        } else if (op == kPropRemove) {
            if (v.size() > 1) {
                v.erase(labelIt);
                gMe->Styles().erase(stylesIt);
            }
            return true;
        } else if (op == kPropInsert) {
            UILabel::LabelStyle labelStyle(v.Owner());
            if (PropSync(labelStyle, val, prop, i, op)) {
                if (v.size() < 8) {
                    v.insert(labelIt, labelStyle);
                    RndText::Style textStyle = gMe->Styles().Owner();
                    gMe->Styles().insert(stylesIt, textStyle);
                }
                return true;
            }
        }
        return false;
    }
}

BEGIN_PROPSYNCS(UILabel)
    SYNC_PROP_SET(text_token, TextToken(), SetTextToken(_val.ForceSym()))
    SYNC_PROP_SET(icon, &unk120, SetIcon(_val.Str(0)[0]))
    SYNC_PROP_SET(edit_text, unk118.c_str(), SetEditText(_val.Str()))
    SYNC_PROP_MODIFY(width, mWidth, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(height, mHeight, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(circle, mCircle, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(alignment, (int &)mAlignment, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(fit_type, (int &)mFitType, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(caps_mode, (int &)mCapsMode, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(markup, mMarkup, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_delay, mScrollDelay, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_rate, mScrollRate, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(scroll_pause, mScrollPause, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(leading, mLeading, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(indentation, mIndentation, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_MODIFY(basic_markup, mBasicMarkup, if (!sDeferUpdate) LabelUpdate(false))
    SYNC_PROP_SET(
        fixed_length, mFixedLength, SetFixedLength(_val.Int());
        if (!sDeferUpdate) LabelUpdate(false)
    )
    SYNC_PROP(draw_width, mDrawRect.w)
    gMe = this;
    SYNC_PROP(styles, mLabelStyles)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(UILabel)
    SAVE_REVS(0x21, 1)
    SAVE_SUPERCLASS(UIComponent)
    bs << mTextToken;
    if (bs.Cached() && !AllowEditText()) {
        bs << gNullStr;
    } else {
        bs << unk118;
    }
    bs << unk120;
    bs << mAlignment;
    bs << mWidth;
    bs << mLeading;
    bs << mFixedLength;
    bs << mMarkup;
    bs << mCapsMode;
    bs << mHeight;
    bs << mCircle;
    bs << mFitType;
    bs << mLabelStyles.size();
    for (int i = 0; i < mLabelStyles.size(); i++) {
        LabelStyle &curLabelStyle = mLabelStyles[i];
        bs << curLabelStyle.mFontResource;
        bs << curLabelStyle.mColorOverride;
        RndText::Style &curStyle = Style(i);
        bs << curStyle.mInfo.mSize;
        bs << curStyle.mInfo.mKerning;
        bs << curStyle.mInfo.mZOffset;
        bs << curStyle.mInfo.mItalics;
        bs << curStyle.mInfo.mFontColor.alpha;
        bs << curStyle.mBlacklight;
    }
    bs << mScrollDelay;
    bs << mScrollRate;
    bs << mScrollPause;
    bs << mIndentation;
    bs << mBasicMarkup;
    for (int i = 0; i < mLabelStyles.size(); i++) {
        bs << GetFontMat(i);
    }
END_SAVES

BEGIN_COPYS(UILabel)
    COPY_SUPERCLASS(UIComponent)
    COPY_SUPERCLASS(RndText)
    CREATE_COPY(UILabel)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTextToken)
        COPY_MEMBER(unk118)
        // isn't this just one char? why strcpy it?
        strcpy(&unk120, &c->unk120);
        COPY_MEMBER(mLabelStyles)
    END_COPYING_MEMBERS
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
END_COPYS

void UILabel::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

INIT_REVS(0x21, 1)

void UILabel::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0x21, 1)
    UIComponent::PreLoad(d.stream);
    if (d.rev > 0x1B) {
        d >> mTextToken;
        d >> unk118;
        d >> unk120;
        d >> (int &)mAlignment;
        d >> mWidth;
        d >> mLeading;
        int len;
        d >> len;
        SetFixedLength(len);
        d >> mMarkup;
        d >> (int &)mCapsMode;
        d >> mHeight;
        if (d.altRev > 0) {
            d >> mCircle;
        }
        d >> (int &)mFitType;
        int numLabelStyles;
        d >> numLabelStyles;
        mLabelStyles.resize(numLabelStyles);
        mStyles.resize(numLabelStyles);
        for (int i = 0; i < mLabelStyles.size(); i++) {
            LabelStyle &curLabelStyle = mLabelStyles[i];
            d >> curLabelStyle.mFontResource;
            d >> curLabelStyle.mColorOverride;
            RndText::Style &curStyle = Style(i);
            d >> curStyle.mInfo.mSize;
            d >> curStyle.mInfo.mKerning;
            d >> curStyle.mInfo.mZOffset;
            d >> curStyle.mInfo.mItalics;
            d >> curStyle.mInfo.mFontColor.alpha;
            if (d.rev >= 0x1E) {
                d >> curStyle.mBlacklight;
            }
        }
        if (d.rev >= 0x1F) {
            d >> mScrollDelay;
            d >> mScrollRate;
            d >> mScrollPause;
        }
        if (d.rev >= 0x20) {
            d >> mIndentation;
        }
        if (d.rev >= 0x21) {
            d >> mBasicMarkup;
        }
    } else {
        if (d.rev > 0 && d.rev < 0xE) {
            bool b;
            d >> b;
        }
        d >> mTextToken;
        if (d.rev > 0xD) {
            d >> unk118;
        }
        if (d.rev > 0xE) {
            if (d.rev < 0x19) {
                String str;
                d >> str;
                unk120 = str.c_str()[0];
            } else {
                d >> unk120;
            }
        }
        if (d.rev > 1) {
            d >> Style(0).mInfo.mSize;
            d >> (int &)mAlignment;
            d >> (int &)mCapsMode;
            if (d.rev > 7) {
                d >> mMarkup;
            }
            d >> mLeading;
            d >> Style(0).mInfo.mKerning;
        }
        if (d.rev > 4) {
            d >> Style(0).mInfo.mItalics;
        }
        if (d.rev > 2) {
            d >> (int &)mFitType;
            d >> mWidth;
            d >> mHeight;
        }
        if (d.rev < 4) {
            Transform &xfm = DirtyLocalXfm();
            if (mAlignment & 1) {
                xfm.v.x -= mWidth / 2.0f;
            } else if (mAlignment & 4) {
                xfm.v.x += mWidth / 2.0f;
            }
            if (mAlignment & 0x10) {
                xfm.v.z += mHeight / 2.0f;
            } else if (mAlignment & 0x40) {
                xfm.v.z -= mHeight / 2.0f;
            }
        }
        if (d.rev > 5) {
            int len;
            d >> len;
            SetFixedLength(len);
        }
        if (d.rev > 6 && d.rev < 0x1B) {
            int x;
            d >> x;
        }
        if (d.rev > 8 && d.rev < 0x10) {
            bool b;
            int x, y, z;
            d >> b >> z >> x >> y;
        }
        if (d.rev > 9 && d.rev < 0x1A) {
            String str;
            d >> str;
        }
        if (d.rev > 10) {
            d >> Style(0).mInfo.mFontColor.alpha;
        }
        if (d.rev > 0xC) {
            d >> LStyle(0).mColorOverride;
        }
        if (d.rev > 0x10 && d.rev < 0x1D) {
            bool b;
            d >> b;
        }
        if (d.rev > 0x11) {
            float size;
            d >> size;
            ObjPtr<UIColor> color(this);
            d >> color;
            bool b2c4;
            d >> b2c4;
            int i9 = (b2c4 == 0) ? 2 : 1;
            if (b2c4) {
                FilePath fp = mLabelStyles[0].mFontResource.GetFile();
                mLabelStyles.resize(i9);
                mLabelStyles[0].mFontResource.LoadFile(fp, true, true, kLoadFront, false);
                mStyles.resize(i9);
            }
            Style(1).mInfo.mSize = size;
            LStyle(1).mColorOverride = color;
        }
        if (d.rev > 0x12) {
            d >> Style(1).mInfo.mKerning;
        } else {
            Style(1).mInfo.mKerning = Style(0).mInfo.mKerning;
        }
        if (d.rev > 0x13) {
            d >> Style(1).mInfo.mZOffset;
            if (d.rev < 0x19) {
                Style(1).mInfo.mZOffset /= Style(1).mInfo.mSize;
            }
        }
        if (d.rev > 0x14) {
            Symbol s;
            d >> s;
            d.stream.PushRev((int)s, this);
        }
        if (d.rev > 0x15) {
            char name[256];
            if (mLabelStyles.size() == 2) {
                LabelStyle &style = LStyle(1);
                d.stream.ReadString(name, 256);
                style.mFontResource.SetName(name, true);
            } else {
                d.stream.ReadString(name, 256);
            }
        }
        if (d.rev > 0x16) {
            Symbol s;
            d >> s;
            d.stream.PushRev((int)s, this);
        }
        if (d.rev > 0x17) {
            d >> Style(1).mInfo.mItalics;
            d >> Style(1).mInfo.mFontColor.alpha;
        }
    }
    d.PushRev(this);
}

void UILabel::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    for (int i = 0; i < mLabelStyles.size(); i++) {
        mLabelStyles[i].mFontResource.PostLoad(nullptr);
    }
    if (d.rev >= 0x1C) {
        for (int i = 0; i < mLabelStyles.size(); i++) {
            char name[256];
            d.stream.ReadString(name, 256);
            SetFontMat(name, i);
        }
    } else if (d.rev > 0x14) {
        if (d.rev > 0x16) {
            SetFontMat((const char *)d.stream.PopRev(this), 1);
        }
        SetFontMat((const char *)d.stream.PopRev(this), 0);
    } else {
        for (int i = 0; i < mLabelStyles.size(); i++) {
            SetFontMat("", i);
        }
    }
    UIComponent::PostLoad(d.stream);
    sDeferUpdate = true;
    if (unk120 != 0) {
        SetText(&unk120);
    } else if (unk118.empty() || (!TheLoadMgr.EditMode() && !AllowEditText())) {
        SetTextToken(mTextToken);
    } else {
        SetText(unk118.c_str());
    }
    if (sRequireFixedLength && mFixedLength == 0) {
        MILO_NOTIFY(
            "%s: %s is preloaded, but doesn't have fixed length", PathName(Dir()), Name()
        );
    }
    sDeferUpdate = false;
    if (mTextToken.Null() && unk120 == 0 && mFixedLength == 0) {
        unk122 = true;
    } else {
        LabelUpdate(false);
    }
}

void UILabel::Highlight() {
    RndTransformable::Highlight();
    Box box;
    GetWidthHeightBox(box);
    Hmx::Color c(1.0f, 1.0f, 0.5f);
    if (!CheckValid(false)) {
        int secs = TheTaskMgr.UISeconds() * 2.0f;
        if (secs % 2 == 0) {
            c.Set(1.0f, 0.2f, 0.2f, 1.0f);
        }
    }
    RndText::Highlight();
    UtilDrawBox(WorldXfm(), box, c, false);
}

void UILabel::SetTextToken(Symbol s) {
    mTextToken = s;
    if (TheLoadMgr.EditMode()) {
        if (!unk118.empty()) {
            return;
        }
        if (unk120 != 0) {
            return;
        }
    }
    SetTokenFmtImp(mTextToken, 0, 0, 0, true);
}

void UILabel::SetInt(int i, bool b) {
    if (b) {
        SetDisplayText(LocalizeSeparatedInt(i, TheLocale), true);
    } else
        SetDisplayText(MakeString("%d", i), true);
}

void UILabel::DrawShowing() {
    if (Style(0).mInfo.mFontColor.alpha > 0) {
        if (unk122 && !sDeferUpdate) {
            LabelUpdate(false);
        }
        MILO_ASSERT(mLabelStyles.size() == mStyles.size(), 0x1EF);
        UILabelDir *rsrc = mLabelStyles[0].mFontResource;
        if (rsrc) {
            UIColor *color = rsrc->GetStateColor(mState);
            for (int i = 0; i < mLabelStyles.size(); i++) {
                LabelStyle &curLabelStyle = mLabelStyles[i];
                RndText::Style &curStyle = Style(i);
                curStyle.mInfo.mFontColorOverride = true;
                UIColor *curColor = curLabelStyle.mColorOverride;
                if (!curColor) {
                    curColor = color;
                }
                const Hmx::Color &curColorColor = curColor->GetColor();
                curStyle.mInfo.mFontColor.red = curColorColor.red;
                curStyle.mInfo.mFontColor.green = curColorColor.green;
                curStyle.mInfo.mFontColor.blue = curColorColor.blue;
            }
        }
        RndText::DrawShowing();
        if (sDebugHighlight && !sInDebugHighlight) {
            sInDebugHighlight = true;
            Highlight();
            sInDebugHighlight = false;
        }
    }
}

void UILabel::OldResourcePreload(BinStream &bs) {
    char buf[0x100];
    ResourceDirPtr<UILabelDir> &rsrc = LStyle(0).mFontResource;
    bs.ReadString(buf, 0x100);
    rsrc.SetName(buf, true);
}

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
    if (unk120 == '\0' && TheLoadMgr.EditMode()) {
        SetEditText(unk118.c_str());
    } else {
        SetDisplayText(&unk120, !TheLoadMgr.EditMode());
    }
}

RndText::Style &UILabel::Style(int idx) {
    if (idx < mStyles.size()) {
        return mStyles[idx];
    } else {
        static RndText::Style s(nullptr);
        return s;
    }
}

const RndText::Style &UILabel::Style(int idx) const {
    if (idx < mStyles.size()) {
        return mStyles[idx];
    } else {
        static RndText::Style s(nullptr);
        return s;
    }
}

UILabel::LabelStyle &UILabel::LStyle(int idx) {
    if (idx < mLabelStyles.size()) {
        return mLabelStyles[idx];
    } else {
        static LabelStyle s(nullptr);
        return s;
    }
}

const UILabel::LabelStyle &UILabel::LStyle(int idx) const {
    if (idx < mLabelStyles.size()) {
        return mLabelStyles[idx];
    } else {
        static LabelStyle s(nullptr);
        return s;
    }
}

void UILabel::SetTokenFmt(const DataArray *da) {
    da->Evaluate(0);
    bool b = da->Size() > 1 && da->Evaluate(1).Type() == kDataArray;
    if (b) {
        SetTokenFmtImp(da->ForceSym(0), da->Array(1), da, 2, false);
    } else {
        SetTokenFmtImp(da->ForceSym(0), 0, da, 1, false);
    }
}

void UILabel::SetPrelocalizedString(String &s) { SetDisplayText(s.c_str(), true); }

void UILabel::SetSubtitle(const DataArray *a) { SetDisplayText(a->Str(2), true); }

void UILabel::SetTimeHMS(int i1, bool b2) {
    int i28 = Min(99, i1 / 3600);
    int i2c = Min(99, i1 / 0x3c + i28 * -0x3c);
    int i30 = Min(99, i1 + (i28 * 0x3c + i2c) * -0x3c);
    if (i28 > 0 || b2) {
        SetDisplayText(MakeString("%02d:%02d:%02d", i28, i2c, i30), true);
    } else {
        SetDisplayText(MakeString("%d:%02d", i2c, i30), true);
    }
}

bool UILabel::CheckValid(bool notify) {
    if (mFixedLength == 0 || UTF8StrLen(mText.c_str()) <= mFixedLength) {
        return true;
    } else {
        if (notify) {
            MILO_NOTIFY(
                "%s: %s has fixed length of %i but text is %i long (%s)",
                PathName(Dir()),
                Name(),
                mFixedLength,
                UTF8StrLen(mText.c_str()),
                mText
            );
        }
        return false;
    }
}

void UILabel::SetEditText(const char *c) {
    if (!TheLoadMgr.EditMode()) {
        if (!AllowEditText()) {
            MILO_FAIL(
                "Called SetEditText, not in milo and type %s does not allow edit text",
                Type()
            );
        }
    }
    unk118 = c;
    if (!unk120) {
        if (unk118.empty()) {
            SetTextToken(mTextToken);
        } else {
            char buf[256];
            ASCIItoUTF8(buf, 256, c);
            SetDisplayText(buf, !TheLoadMgr.EditMode());
        }
    }
}

char const *UILabel::GetDefaultText() const {
    if (unk120 != 0) {
        return &unk120;
    }

    if (TheLoadMgr.EditMode() && !unk118.empty())
        return unk118.c_str();
    else
        return Localize(mTextToken, nullptr, TheLocale);
}

void UILabel::CenterWithLabel(UILabel *label, bool b2, float f3) {
    MILO_ASSERT((mAlignment & RndText::kCenter) || (label->mAlignment & RndText::kCenter), 0x400);
    Transform myXfm = LocalXfm();
    Transform labelXfm = label->LocalXfm();
    int add = b2 ? 1 : -1;
    myXfm.v.x = label->mDrawRect.w * 0.5f + f3 * 0.5f * (float)add + labelXfm.v.x;
    labelXfm.v.x = -(label->mDrawRect.w * 0.5f + f3 * 0.5f * (float)add - labelXfm.v.x);
    SetLocalXfm(myXfm);
    label->SetLocalXfm(labelXfm);
}

void UILabel::Init() {
    REGISTER_OBJ_FACTORY(UILabel);
    UILabelDir::Init();
}

bool UILabel::AllowEditText() const {
    if (TheUI->DefaultAllowEditText()) {
        return true;
    } else if (LStyle(0).mFontResource) {
        UILabelDir *rsrc = LStyle(0).mFontResource;
        return rsrc->AllowEditText();
    } else {
        MILO_NOTIFY("LabelDir is not yet loaded, can't tell if edit text is allowed");

        return false;
    }
}

void UILabel::LabelUpdate(bool b) {
    unk122 = false;
    RndFontBase *font = Style(0).mFont;
    Style(0).mInfo.mTextColor.Set(1, 1, 1, 1);
    for (int i = 1; i < mLabelStyles.size(); i++) {
        RndText::Style &curStyle = Style(i);
        LabelStyle &curLabelStyle = LStyle(i);
        if (curLabelStyle.mColorOverride && curStyle.mFont && curStyle.mFont != font) {
            curStyle.mInfo.mTextColor = curLabelStyle.mColorOverride->GetColor();
        } else {
            curStyle.mInfo.mTextColor.Set(1, 1, 1, 1);
        }
    }
    UpdateText();
    CheckValid(!TheLoadMgr.EditMode());
}

void UILabel::SetFontMat(char const *c, int idx) {
    RndFontBase *font = nullptr;
    UILabelDir *fontRsrc = LStyle(idx).mFontResource;
    if (fontRsrc) {
        font = fontRsrc->FontObj(c);
        if (!font) {
            if (*c != '\0') {
                MILO_NOTIFY(
                    "%s is referencing a mat variation '%s' that no longer exists, trying default...",
                    PathName(this),
                    c
                );
                font = fontRsrc->FontObj("");
            }
            if (!font) {
                MILO_NOTIFY(
                    "%s in resource %s has no default mat variation",
                    PathName(this),
                    PathName(fontRsrc)
                );
            }
        }
    } else {
        if (*c != '\0') {
            MILO_NOTIFY(
                "%s [styles 0 font_resource] is NULL, can't set fontmat %s",
                PathName(this),
                c
            );
        }
    }
    if (idx < mStyles.size()) {
        mStyles[idx].mFont = font;
    }
}

const char *UILabel::GetFontMat(int idx) {
    RndFontBase *font = nullptr;
    if (idx < mStyles.size()) {
        font = mStyles[idx].mFont;
    }
    UILabelDir *fontRsrc = LStyle(idx).mFontResource;
    return fontRsrc ? fontRsrc->GetMatVariationName(font) : "";
}

void UILabel::RefreshFontMat(int i) {
    auto mat = GetFontMat(i);
    SetFontMat(mat, i);
    if (sDeferUpdate == false) {
        LabelUpdate(false);
    }
}

DataNode UILabel::OnSetPrelocalizedString(const DataArray *a) {
    const DataNode &stringNode = a->Evaluate(2);
    MILO_ASSERT(stringNode.Type() == kDataString, 0x386);
    String str(stringNode.Str());
    SetPrelocalizedString(str);
    return 1;
}

DataNode UILabel::OnSetTokenFmt(const DataArray *da) {
    const DataNode &n = da->Evaluate(2);
    if (n.Type() == kDataArray) {
        DataArray *arr = n.Array();
        bool b = arr->Size() > 1 && arr->Evaluate(1).Type() == kDataArray;
        if (b) {
            SetTokenFmtImp(arr->ForceSym(0), arr->Array(1), arr, 2, false);
        } else
            SetTokenFmtImp(arr->ForceSym(0), 0, arr, 1, false);
    } else {
        bool b = da->Size() > 3 && da->Evaluate(3).Type() == kDataArray;
        if (b) {
            SetTokenFmtImp(da->ForceSym(2), da->Array(3), da, 4, false);
        } else {
            SetTokenFmtImp(da->ForceSym(2), 0, da, 3, false);
        }
    }
    return 1;
}

DataNode UILabel::OnSetInt(const DataArray *da) {
    int num;
    if (da->Type(2) == kDataFloat) {
        num = da->Float(2);
    } else {
        num = da->Int(2);
    }
    bool b = false;
    if (da->Size() > 3) {
        b = da->Int(3);
    }
    SetInt(num, b);
    return 1;
}

DataNode UILabel::OnSetTimeHMS(const DataArray *da) {
    int num;
    if (da->Type(2) == kDataFloat) {
        num = da->Float(2);
    } else {
        num = da->Int(2);
    }
    SetTimeHMS(num, true);
    return 1;
}

DataNode UILabel::OnSetHeightFromText(DataArray *a) {
    if (mFitType == 0 && Style(0).mFont) {
        float fl;
        mHeight = ComputeHeight(unkc4, 1.0f, fl);
    } else {
        MILO_NOTIFY(
            "Could not set height, either no default font set, or fit type is not kFitWrap"
        );
    }
    return 0;
}

void UILabel::SetTokenFmtImp(
    Symbol s, DataArray const *d1, DataArray const *d2, int i, bool b
) {
    mTextToken = s;
    if (s.Null()) {
        SetDisplayText(gNullStr, true);
    } else {
        bool found;
        const char *localize = Localize(mTextToken, &found, TheLocale);
        if (found) {
            // well SuperFormatString is called here but that doesnt exist yet
        } else {
            SetDisplayText(localize, false);
        }
    }
}

float GetPctHeightFromTextSize(float f) {
    if (TheLoadMgr.EditMode()) {
        Vector3 vec3a(0, 0, 0);
        Vector2 vec2a;
        TheUI->GetCam()->WorldToScreen(vec3a, vec2a);
        Vector3 vec3b(0, 0, -f);
        Vector2 vec2b;
        TheUI->GetCam()->WorldToScreen(vec3b, vec2b);
        return fabs(vec2a.y - vec2b.y);
    }
    return f;
}

float GetTextSizeFromPctHeight(float f) {
    if (TheLoadMgr.EditMode()) {
        float val = -TheUI->GetCam()->LocalXfm().v.y;
        Vector2 vec2a(0, 0);
        Vector3 vec3a;
        TheUI->GetCam()->ScreenToWorld(vec2a, val, vec3a);
        Vector2 vec2b(0, f);
        Vector3 vec3b;
        TheUI->GetCam()->ScreenToWorld(vec2b, val, vec3b);
        return fabs(vec3a.z - vec3b.z);
    }
    return f;
}
