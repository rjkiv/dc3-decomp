#include "ui/UIListWidget.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Utl.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "math/Vec.h"
#include "utl/BinStream.h"

UIListWidget::UIListWidget()
    : mDrawOrder(0), mDisabledAlphaScale(1), mDefaultColor(this),
      mWidgetDrawType(kUIListWidgetDrawAlways), mParentList(nullptr) {
    for (int i = 0; i < kNumUIListWidgetStates; i++) {
        std::vector<ObjPtr<UIColor> > vec;
        for (int j = 0; j < UIComponent::kNumStates; j++) {
            vec.push_back(ObjPtr<UIColor>(this));
        }
        mColors.push_back(vec);
    }
}

BEGIN_HANDLERS(UIListWidget)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(UIListWidget)
    SYNC_PROP(draw_order, mDrawOrder)
    SYNC_PROP(disabled_alpha_scale, mDisabledAlphaScale)
    SYNC_PROP(default_color, mDefaultColor)
    SYNC_PROP_SET(
        widget_draw_type,
        (int &)mWidgetDrawType,
        mWidgetDrawType = (UIListWidgetDrawType)_val.Int()
    )
    SYNC_PROP_SET(
        active_normal_color,
        mColors[kUIListWidgetActive][UIComponent::kNormal].Ptr(),
        SetColor(kUIListWidgetActive, UIComponent::kNormal, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        active_focused_color,
        mColors[kUIListWidgetActive][UIComponent::kFocused].Ptr(),
        SetColor(kUIListWidgetActive, UIComponent::kFocused, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        active_disabled_color,
        mColors[kUIListWidgetActive][UIComponent::kDisabled].Ptr(),
        SetColor(kUIListWidgetActive, UIComponent::kDisabled, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        active_selecting_color,
        mColors[kUIListWidgetActive][UIComponent::kSelecting].Ptr(),
        SetColor(kUIListWidgetActive, UIComponent::kSelecting, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        active_selected_color,
        mColors[kUIListWidgetActive][UIComponent::kSelected].Ptr(),
        SetColor(kUIListWidgetActive, UIComponent::kSelected, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        highlight_normal_color,
        mColors[kUIListWidgetHighlight][UIComponent::kNormal].Ptr(),
        SetColor(kUIListWidgetHighlight, UIComponent::kNormal, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        highlight_focused_color,
        mColors[kUIListWidgetHighlight][UIComponent::kFocused].Ptr(),
        SetColor(kUIListWidgetHighlight, UIComponent::kFocused, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        highlight_disabled_color,
        mColors[kUIListWidgetHighlight][UIComponent::kDisabled].Ptr(),
        SetColor(kUIListWidgetHighlight, UIComponent::kDisabled, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        highlight_selecting_color,
        mColors[kUIListWidgetHighlight][UIComponent::kSelecting].Ptr(),
        SetColor(kUIListWidgetHighlight, UIComponent::kSelecting, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        highlight_selected_color,
        mColors[kUIListWidgetHighlight][UIComponent::kSelected].Ptr(),
        SetColor(kUIListWidgetHighlight, UIComponent::kSelected, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        inactive_normal_color,
        mColors[kUIListWidgetInactive][UIComponent::kNormal].Ptr(),
        SetColor(kUIListWidgetInactive, UIComponent::kNormal, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        inactive_focused_color,
        mColors[kUIListWidgetInactive][UIComponent::kFocused].Ptr(),
        SetColor(kUIListWidgetInactive, UIComponent::kFocused, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        inactive_disabled_color,
        mColors[kUIListWidgetInactive][UIComponent::kDisabled].Ptr(),
        SetColor(kUIListWidgetInactive, UIComponent::kDisabled, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        inactive_selecting_color,
        mColors[kUIListWidgetInactive][UIComponent::kSelecting].Ptr(),
        SetColor(kUIListWidgetInactive, UIComponent::kSelecting, _val.Obj<UIColor>())
    )
    SYNC_PROP_SET(
        inactive_selected_color,
        mColors[kUIListWidgetInactive][UIComponent::kSelected].Ptr(),
        SetColor(kUIListWidgetInactive, UIComponent::kSelected, _val.Obj<UIColor>())
    )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(UIListWidget)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mDrawOrder << mDefaultColor << mWidgetDrawType;
    bs << mDisabledAlphaScale;
    for (int i = 0; i < kNumUIListWidgetStates; i++) {
        for (int j = 0; j < UIComponent::kNumStates; j++) {
            bs << mColors[i][j];
        }
    }
END_SAVES

BEGIN_COPYS(UIListWidget)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(UIListWidget, w)
    MILO_ASSERT(w, 0xc7);
    COPY_MEMBER_FROM(w, mDrawOrder)
    COPY_MEMBER_FROM(w, mDisabledAlphaScale)
    COPY_MEMBER_FROM(w, mDefaultColor)
    COPY_MEMBER_FROM(w, mColors)
    COPY_MEMBER_FROM(w, mWidgetDrawType)
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(UIListWidget)
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mDrawOrder;
    if (d.rev < 1) {
        int i, j;
        d >> i >> j;
    }
    d >> mDefaultColor;
    int x;
    d >> x;
    mWidgetDrawType = (UIListWidgetDrawType)x;
    if (d.rev >= 2) {
        d >> mDisabledAlphaScale;
    }
    for (int i = 0; i < kNumUIListWidgetStates; i++) {
        for (int j = 0; j < UIComponent::kNumStates; j++) {
            ObjPtr<UIColor> color(this);
            d >> color;
            mColors[i][j] = color;
        }
    }
END_LOADS

void UIListWidget::ResourceCopy(const UIListWidget *w) { Copy(w, kCopyShallow); }

float UIListWidget::DrawOrder() const { return mDrawOrder; }

void UIListWidget::CalcXfm(const Transform &tfin, const Vector3 &vin, Transform &out) {
    out.v.x += vin.x;
    out.v.y += vin.y;
    out.v.z += vin.z;
    Multiply(out, tfin, out);
}

void UIListWidget::SetParentList(UIList *list) { mParentList = list; }

void UIListWidget::SetColor(UIListWidgetState ws, UIComponent::State cs, UIColor *color) {
    MILO_ASSERT(ws < kNumUIListWidgetStates, 0x7C);
    MILO_ASSERT(cs < UIComponent::kNumStates, 0x7D);
    mColors[ws][cs] = color;
}

UIColor *UIListWidget::DisplayColor(
    UIListWidgetState element_state, UIComponent::State list_state
) const {
    MILO_ASSERT_RANGE(element_state, 0, kNumUIListWidgetStates, 0x64);
    MILO_ASSERT_RANGE(list_state, 0, UIComponent::kNumStates, 0x65);
    UIColor *color = mColors[element_state][list_state];
    if (color)
        return color;
    else if (mDefaultColor)
        return mDefaultColor;
    else
        return nullptr;
}

void UIListWidget::DrawMesh(
    RndMesh *mesh,
    UIListWidgetState wstate,
    UIComponent::State cstate,
    const Transform &tf,
    Box *box
) {
    MILO_ASSERT(mesh, 0x40);
    mesh->SetWorldXfm(tf);
    if (box) {
        Box localbox(box->mMin, box->mMax);
        CalcBox(mesh, localbox);
        box->GrowToContain(localbox.mMin, false);
        box->GrowToContain(localbox.mMax, false);
    } else {
        UIColor *col = DisplayColor(wstate, cstate);
        if (col) {
            RndMat *mat = mesh->Mat();
            if (mat) {
                const Hmx::Color &uiColor = col->GetColor();
                mat->SetColor(uiColor.red, uiColor.green, uiColor.blue);
            }
        }
        mesh->DrawShowing();
    }
}

UIList *UIListWidget::ParentList() { return mParentList; }
float UIListWidget::DisabledAlphaScale() const { return mDisabledAlphaScale; }
UIListWidgetDrawType UIListWidget::DrawType() const { return mWidgetDrawType; }
