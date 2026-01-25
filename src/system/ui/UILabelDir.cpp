#include "ui/UILabelDir.h"
#include "UIColor.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "rndobj/FontBase.h"
#include "ui/UIComponent.h"
#include "ui/UIFontImporter.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

UILabelDir::UILabelDir()
    : mDefaultColor(this), mFocusAnim(this), mPulseAnim(this),
      mFocusedBackgroundGroup(this), mUnfocusedBackgroundGroup(this),
      mAllowEditText(false) {
    for (int i = 0; i < UIComponent::kNumStates; i++) {
        mColors.push_back(ObjPtr<UIColor>(this));
    }
}

BEGIN_PROPSYNCS(UILabelDir)
    SYNC_PROP(allow_edit_text, mAllowEditText)
    SYNC_PROP(focus_anim, mFocusAnim)
    SYNC_PROP(pulse_anim, mPulseAnim)
    SYNC_PROP(focused_background_group, mFocusedBackgroundGroup)
    SYNC_PROP(unfocused_background_group, mUnfocusedBackgroundGroup)
    SYNC_PROP(default_color, mDefaultColor)
    SYNC_PROP_SET(
        normal_color,
        (Hmx::Object *)mColors[UIComponent::kNormal],
        mColors[UIComponent::kNormal] = _val.Obj<UIColor>()
    )
    SYNC_PROP_SET(
        focused_color,
        (Hmx::Object *)mColors[UIComponent::kFocused],
        mColors[UIComponent::kFocused] = _val.Obj<UIColor>()
    )
    SYNC_PROP_SET(
        disabled_color,
        (Hmx::Object *)mColors[UIComponent::kDisabled],
        mColors[UIComponent::kDisabled] = _val.Obj<UIColor>()
    )
    SYNC_PROP_SET(
        selecting_color,
        (Hmx::Object *)mColors[UIComponent::kSelecting],
        mColors[UIComponent::kSelecting] = _val.Obj<UIColor>()
    )
    SYNC_PROP_SET(
        selected_color,
        (Hmx::Object *)mColors[UIComponent::kSelected],
        mColors[UIComponent::kSelected] = _val.Obj<UIColor>()
    )
    SYNC_SUPERCLASS(UIFontImporter)
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(UILabelDir)
END_SAVES

BEGIN_COPYS(UILabelDir)
    COPY_SUPERCLASS(RndDir)
    COPY_SUPERCLASS(UIFontImporter)
    CREATE_COPY(UILabelDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDefaultColor)
        COPY_MEMBER(mColors)
        COPY_MEMBER(mAllowEditText)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(UILabelDir)
    ObjectDir::Load(bs);
END_LOADS

void UILabelDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(0xb, 0);
    RndDir::PreLoad(bs);
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void UILabelDir::PostLoad(BinStream &bs) {}

bool UILabelDir::AllowEditText() const { return mAllowEditText; }

RndFontBase *UILabelDir::FontObj(Symbol s) const { return nullptr; }

UIColor *UILabelDir::GetStateColor(UIComponent::State state) const {
    MILO_ASSERT(state < UIComponent::kNumStates, 0x39);
    return nullptr;
}

DataNode UILabelDir::GetMatVariations(UILabelDir *) { return NULL_OBJ; }

void UILabelDir::Init() {}

BEGIN_HANDLERS(UILabelDir)
    HANDLE_EXPR(font_obj, FontObj(_msg->Sym(2)))
    HANDLE_SUPERCLASS(UIFontImporter)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS
