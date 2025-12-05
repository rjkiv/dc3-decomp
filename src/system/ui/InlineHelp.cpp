#include "ui/InlineHelp.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Joypad.h"
#include "rndobj/Dir.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"
#include "utl/BinStream.h"
#include "utl/Locale.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region InlineHelp

InlineHelp::InlineHelp()
    : mUseConnectedControllers(false), mHorizontal(true), mSpacing(0), mResourceDir(this),
      unk88(0), mTextColor(this) {}

InlineHelp::~InlineHelp() {
    int siz = mTextLabels.size();
    for (int i = 0; i < siz; i++) {
        delete mTextLabels[i];
    }
}

BEGIN_LOADS(InlineHelp)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

BEGIN_SAVES(InlineHelp)
    SAVE_REVS(5, 0)
    bs << mHorizontal;
    bs << mSpacing;
    // bs << mConfig;
    bs << mResourceDir;
    bs << mUseConnectedControllers;
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(InlineHelp)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY_AS(InlineHelp, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mHorizontal)
        COPY_MEMBER(mSpacing)
        COPY_MEMBER(mConfig)
        COPY_MEMBER(mTextColor)
        COPY_MEMBER(mUseConnectedControllers)
        COPY_MEMBER(mResourceDir)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_PROPSYNCS(InlineHelp)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_PROP_MODIFY(config, mConfig, SyncLabelsToConfig())
    SYNC_PROP(horizontal, mHorizontal)
    SYNC_PROP(spacing, mSpacing)
    SYNC_PROP_MODIFY(text_color, mTextColor, UpdateTextColors())
    SYNC_PROP(use_connected_controllers, mUseConnectedControllers)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

void InlineHelp::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    // mResourceDir->PostLoad(bs);
    UIComponent::PostLoad(bs);
    Update();
}

void InlineHelp::UpdateLabelText() {
    static Symbol inline_help_fmt("inline_help_fmt");
    int size = mConfig.size();
    for (int i = 0; i < size; i++) {
        String icon = GetIconStringFromAction(mConfig[i].mAction);
        if (icon.empty())
            mTextLabels[i]->SetTextToken(gNullStr);
        else
            mTextLabels[i]->SetTokenFmt(
                inline_help_fmt, icon.c_str(), mConfig[i].GetText(sRotated)
            );
    }
}
void InlineHelp::Init() { REGISTER_OBJ_FACTORY(InlineHelp) }

void InlineHelp::Enter() {
    UIComponent::Enter();
    UpdateIconTypes(true);
    SyncLabelsToConfig();
}

void InlineHelp::SetTypeDef(DataArray *d) {
    Hmx::Object::SetTypeDef(d);
    Update();
}

String InlineHelp::GetIconStringFromAction(int idx) {
    static Symbol action_chars("action_chars");
    String ret;
    const DataArray *t = TypeDef();
    MILO_ASSERT(t, 0x1cb);
    DataArray *actionArr = t->FindArray(action_chars);
    for (std::vector<Symbol>::iterator it = mIconTypes.begin(); it != mIconTypes.end();
         ++it) {
        const char *str = actionArr->FindArray(*it)->Str(idx + 1);
        char c = *str;
        if (ret.find(c) == String::npos)
            ret += c;
    }
    return ret;
}

void InlineHelp::ResetRotation() {
    sRotated = 0;
    sHasFlippedTextThisRotation = 0;
    sRotationTime = TheTaskMgr.UISeconds() + 5.0f;
    sLabelRot = -0.0f;
}

void InlineHelp::Update() {
    const DataArray *pTypeDef = TypeDef();
    if (pTypeDef && mResourceDir) {
        static Symbol text_label("text_label");
        unk88 = mResourceDir->Find<UILabel>(pTypeDef->FindStr(text_label), true);
        SyncLabelsToConfig();
    }
}

void InlineHelp::UpdateIconTypes(bool b) {
    mIconTypes.clear();
    const DataArray *pTypeDef = TypeDef();
    if (pTypeDef) {
        static Symbol action_chars("action_chars");
        DataArray *charArray = pTypeDef->FindArray(action_chars);
        for (int i = 1; i < charArray->Size(); i++) {
            mIconTypes.push_back(charArray->Array(i)->Sym(0));
        }
    }
}

void InlineHelp::SetLabelRotationPcts(float f) {
    if (f < 0.5f)
        sLabelRot = f * -240.0f;
    else
        sLabelRot = f * -240.0f - 120.0f;
}

void InlineHelp::Poll() {
    UIComponent::Poll();
    float uisecs = TheTaskMgr.UISeconds();
    if (uisecs != sLastUpdatedTime) {
        sNeedsTextUpdate = false;
        if (uisecs > sRotationTime) {
            float f1 = uisecs - sRotationTime;
            if (f1 >= 1.0f) {
                sHasFlippedTextThisRotation = false;
                sRotationTime = uisecs + 5.0f;
                SetLabelRotationPcts(0);
            } else {
                if (!sHasFlippedTextThisRotation && f1 >= 0.5f) {
                    sHasFlippedTextThisRotation = true;
                    sRotated = sRotated == 0;
                    sNeedsTextUpdate = true;
                }
                SetLabelRotationPcts(f1);
            }
        }
        sLastUpdatedTime = uisecs;
    }
    if (sNeedsTextUpdate)
        UpdateLabelText();
}

void InlineHelp::SetActionToken(JoypadAction a, DataNode &node) {
    bool found = false;
    for (std::vector<ActionElement>::iterator it = mConfig.begin(); it != mConfig.end();
         ++it) {
        if ((*it).mAction == a) {
            (*it).SetConfig(node, false);
            found = true;
            break;
        }
    }
    if (!found) {
        ActionElement el(a);
        el.SetConfig(node, false);
        mConfig.push_back(el);
    }
    SyncLabelsToConfig();
}

void InlineHelp::SyncLabelsToConfig() {
    ResetRotation();
    int cfg_size = mConfig.size();
    int labels_size = mTextLabels.size();
    if (cfg_size > labels_size) {
        for (; labels_size < cfg_size; labels_size++) {
            UILabel *lbl = Hmx::Object::New<UILabel>();
            mTextLabels.push_back(lbl);
        }
    } else {
        if (labels_size > cfg_size) {
            for (; cfg_size < labels_size; cfg_size++) {
                delete mTextLabels[cfg_size];
            }
            mTextLabels.resize(cfg_size);
        }
    }
    UpdateLabelText();
}

DataNode InlineHelp::OnSetConfig(const DataArray *da) {
    mConfig.clear();
    DataArray *arr = da->Array(2);
    for (int i = 0; i < arr->Size(); i++) {
        DataArray *loopArr = arr->Array(i);
        ActionElement el((JoypadAction)loopArr->Int(0));
        el.SetConfig(arr->Node(1), false);
        if (loopArr->Size() > 2)
            el.SetConfig(arr->Node(2), true);
        mConfig.push_back(el);
    }
    SyncLabelsToConfig();
    return DataNode(1);
}

BEGIN_HANDLERS(InlineHelp)
    HANDLE_ACTION(
        set_action_token, SetActionToken((JoypadAction)_msg->Int(2), _msg->Node(3))
    )
    HANDLE_ACTION(clear_action_token, ClearActionToken((JoypadAction)_msg->Int(2)))
    HANDLE(set_config, OnSetConfig)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

#pragma endregion InlineHelp
#pragma region InlineHelp::ActionElement

InlineHelp::ActionElement::ActionElement()
    : mAction(kAction_None), mPrimaryToken(gNullStr), mSecondaryToken(gNullStr) {}

InlineHelp::ActionElement::ActionElement(JoypadAction a)
    : mAction(a), mPrimaryToken(gNullStr), mSecondaryToken(gNullStr) {}

InlineHelp::ActionElement::ActionElement(InlineHelp::ActionElement const &ac)
    : mAction(ac.mAction), mPrimaryToken(ac.mPrimaryToken),
      mSecondaryToken(ac.mSecondaryToken), mPrimaryStr(ac.mPrimaryStr),
      mSecondaryStr(ac.mSecondaryStr) {}

InlineHelp::ActionElement::~ActionElement() {}

void InlineHelp::ActionElement::SetToken(Symbol s, bool secondary) {
    if (!secondary) {
        mPrimaryToken = s;
        mPrimaryStr = Localize(s, 0, TheLocale);
    } else {
        mSecondaryToken = s;
        mSecondaryStr = Localize(s, 0, TheLocale);
    }
}

void InlineHelp::ActionElement::SetString(const char *s, bool b) {
    if (!b) {
        mPrimaryToken = gNullStr;
        mPrimaryStr = s;
    } else {
        mSecondaryToken = gNullStr;
        mSecondaryStr = s;
    }
}

void InlineHelp::ActionElement::SetConfig(DataNode &dn, bool b) {
    if (dn.Type() == kDataArray) {
        DataArray *da = dn.Array();
        if (da->Size() == 0)
            return;
        FormatString fs(Localize(da->Sym(0), 0, TheLocale));
        for (int i = 1; i < da->Size(); i++) {
            const DataNode &dn2 = da->Evaluate(i);
            if (dn2.Type() == kDataSymbol) {
                fs << Localize(dn2.Sym(), 0, TheLocale);
            } else {
                fs << dn2;
            }
        }
        SetString(fs.Str(), b);
    } else {
        SetToken(dn.Sym(), b);
    }
}

Symbol InlineHelp::ActionElement::GetToken(bool b) const {
    if (b)
        return mSecondaryToken;
    return mPrimaryToken;
}

BinStream &operator>>(BinStream &bs, InlineHelp::ActionElement &ae) {
    LOAD_REVS(bs);
    {
        int x;
        bs >> x;
        ae.mAction = (JoypadAction)x;
    }
    Symbol s;
    bs >> s;
    ae.SetToken(s, false);
    if (d.rev >= 2) {
        bs >> s;
        ae.SetToken(s, true);
    }
    return bs;
}

const char *InlineHelp::ActionElement::GetText(bool b) const {
    if (b && HasSecondaryStr())
        return mSecondaryStr.c_str();
    return mPrimaryStr.c_str();
}

BEGIN_CUSTOM_PROPSYNC(InlineHelp::ActionElement)
    SYNC_PROP(action, (int &)o.mAction)
    SYNC_PROP_SET(text_token, o.GetToken(false), o.SetToken(_val.Sym(), false))
    SYNC_PROP_SET(secondary_token, o.GetToken(true), o.SetToken(_val.Sym(), true))
END_CUSTOM_PROPSYNC

#pragma endregion InlineHelp::ActionElement
