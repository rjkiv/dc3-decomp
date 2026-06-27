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

float InlineHelp::sLastUpdatedTime = 0;
float InlineHelp::sRotationTime = 0;
float InlineHelp::sLabelRot = 0;
bool InlineHelp::sHasFlippedTextThisRotation = false;
bool InlineHelp::sNeedsTextUpdate = false;
bool InlineHelp::sRotated = false;
const float InlineHelp::sRotateDelay = 5;
const float InlineHelp::sRotateDuration = 1;

#pragma region InlineHelp::ActionElement

InlineHelp::ActionElement::ActionElement()
    : mAction(kAction_None), mPrimaryToken(gNullStr), mSecondaryToken(gNullStr) {}

InlineHelp::ActionElement::ActionElement(JoypadAction a)
    : mAction(a), mPrimaryToken(gNullStr), mSecondaryToken(gNullStr) {}

InlineHelp::ActionElement::~ActionElement() {}

BinStream &operator<<(BinStream &bs, const InlineHelp::ActionElement &a) {
    bs << a.mAction;
    Symbol primary = a.mPrimaryToken;
    bs << primary;
    Symbol secondary = a.mSecondaryToken;
    bs << secondary;
    return bs;
}

BinStream &operator>>(BinStreamRev &d, InlineHelp::ActionElement &a) {
    int action;
    d >> action;
    a.mAction = (JoypadAction)action;
    Symbol token;
    d >> token;
    a.SetToken(token, false);
    if (d.rev >= 2) {
        d >> token;
        a.SetToken(token, true);
    }
    return d.stream;
}

void InlineHelp::ActionElement::SetToken(Symbol token, bool secondary) {
    if (!secondary) {
        mPrimaryToken = token;
        mPrimaryStr = Localize(token, nullptr, TheLocale);
    } else {
        mSecondaryToken = token;
        mSecondaryStr = Localize(token, nullptr, TheLocale);
    }
}

void InlineHelp::ActionElement::SetString(const char *str, bool secondary) {
    if (!secondary) {
        mPrimaryToken = gNullStr;
        mPrimaryStr = str;
    } else {
        mSecondaryToken = gNullStr;
        mSecondaryStr = str;
    }
}

void InlineHelp::ActionElement::SetConfig(DataNode &dn, bool secondary) {
    if (dn.Type() == kDataArray) {
        DataArray *da = dn.Array();
        if (da->Size() != 0) {
            FormatString fs(Localize(da->Sym(0), nullptr, TheLocale));
            for (int i = 1; i < da->Size(); i++) {
                const DataNode &dn2 = da->Evaluate(i);
                if (dn2.Type() == kDataSymbol) {
                    fs << Localize(dn2.Sym(), nullptr, TheLocale);
                } else {
                    fs << dn2;
                }
            }
            SetString(fs.Str(), secondary);
        }
    } else {
        SetToken(dn.Sym(), secondary);
    }
}

Symbol InlineHelp::ActionElement::GetToken(bool secondary) const {
    if (secondary) {
        return mSecondaryToken;
    } else {
        return mPrimaryToken;
    }
}

const char *InlineHelp::ActionElement::GetText(bool secondary) const {
    if (secondary && HasSecondaryStr()) {
        return mSecondaryStr.c_str();
    } else {
        return mPrimaryStr.c_str();
    }
}

BEGIN_CUSTOM_PROPSYNC(InlineHelp::ActionElement)
    SYNC_PROP(action, (int &)o.mAction)
    SYNC_PROP_SET(text_token, o.GetToken(false), o.SetToken(_val.Sym(), false))
    SYNC_PROP_SET(secondary_token, o.GetToken(true), o.SetToken(_val.Sym(), true))
END_CUSTOM_PROPSYNC

#pragma endregion
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

BEGIN_HANDLERS(InlineHelp)
    HANDLE_ACTION(
        set_action_token, SetActionToken((JoypadAction)_msg->Int(2), _msg->Node(3))
    )
    HANDLE_ACTION(clear_action_token, ClearActionToken((JoypadAction)_msg->Int(2)))
    HANDLE(set_config, OnSetConfig)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(InlineHelp)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_PROP_MODIFY(config, mConfig, SyncLabelsToConfig())
    SYNC_PROP(horizontal, mHorizontal)
    SYNC_PROP(spacing, mSpacing)
    SYNC_PROP_MODIFY(text_color, mTextColor, UpdateTextColors())
    SYNC_PROP(use_connected_controllers, mUseConnectedControllers)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(InlineHelp)
    SAVE_REVS(5, 0)
    bs << mHorizontal;
    bs << mSpacing;
    bs << mConfig;
    bs << mTextColor;
    bs << mUseConnectedControllers;
    bs << mResourceDir;
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(InlineHelp)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(InlineHelp)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHorizontal)
        COPY_MEMBER(mSpacing)
        COPY_MEMBER(mConfig)
        COPY_MEMBER(mTextColor)
        COPY_MEMBER(mUseConnectedControllers)
        COPY_MEMBER(mResourceDir)
    END_COPYING_MEMBERS
    Update();
    UpdateIconTypes(false);
END_COPYS

BEGIN_LOADS(InlineHelp)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void InlineHelp::SetTypeDef(DataArray *d) {
    Hmx::Object::SetTypeDef(d);
    Update();
}

INIT_REVS(5, 0)

void InlineHelp::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    d >> mHorizontal;
    d >> mSpacing;
    d >> mConfig;
    if (d.rev >= 1) {
        d >> mTextColor;
    }
    if (d.rev >= 2 && d.rev < 4) {
        int x;
        d >> x;
    }
    if (d.rev >= 3) {
        d >> mUseConnectedControllers;
    }
    if (d.rev >= 5) {
        d >> mResourceDir;
    }
    UIComponent::PreLoad(d.stream);
    d.PushRev(this);
}

void InlineHelp::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
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

void InlineHelp::Enter() {
    UIComponent::Enter();
    UpdateIconTypes(true);
    SyncLabelsToConfig();
}

void InlineHelp::OldResourcePreload(BinStream &bs) {
    char buf[0x100];
    bs.ReadString(buf, 0x100);
    mResourceDir.SetName(buf, true);
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

String InlineHelp::GetIconStringFromAction(int idx) {
    static Symbol action_chars("action_chars");
    String ret;
    const DataArray *t = TypeDef();
    MILO_ASSERT(t, 0x1cb);
    DataArray *actionArr = t->FindArray(action_chars);
    FOREACH (it, mIconTypes) {
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

void InlineHelp::SetActionToken(JoypadAction a, DataNode &node) {
    bool found = false;
    FOREACH (it, mConfig) {
        if (it->mAction == a) {
            it->SetConfig(node, false);
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
            lbl->Copy(unk88, kCopyShallow);
            lbl->LStyle(0).mColorOverride = mTextColor;
            mTextLabels.push_back(lbl);
        }
    } else {
        if (labels_size > cfg_size) {
            for (int i = cfg_size; i < labels_size; i++) {
                delete mTextLabels[i];
            }
            mTextLabels.resize(cfg_size);
        }
    }
    UpdateLabelText();
}

void InlineHelp::UpdateTextColors() {
    FOREACH (it, mTextLabels) {
        (*it)->LStyle(0).mColorOverride = mTextColor;
    }
}

void InlineHelp::ClearActionToken(JoypadAction a) {
    FOREACH (it, mConfig) {
        if (it->mAction == a) {
            mConfig.erase(it);
            SyncLabelsToConfig();
            return;
        }
    }
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
    return 1;
}

#pragma endregion
