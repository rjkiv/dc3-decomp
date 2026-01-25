#include "ui/UIListArrow.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "ui/UIListWidget.h"

UIListArrow::UIListArrow()
    : mMesh(this), mScrollAnim(this), mPosition(kUIListArrowBack), mShowOnlyScroll(0),
      mOnHighlight(0) {}

BEGIN_PROPSYNCS(UIListArrow)
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP(scroll_anim, mScrollAnim)
    SYNC_PROP_SET(position, mPosition, mPosition = (UIListArrowPosition)_val.Int())
    SYNC_PROP(show_only_scroll, mShowOnlyScroll)
    SYNC_PROP(on_highlight, mOnHighlight)
    SYNC_SUPERCLASS(UIListWidget)
END_PROPSYNCS

BEGIN_SAVES(UIListArrow)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(UIListWidget)
    bs << mMesh << mPosition << mShowOnlyScroll << mOnHighlight << mScrollAnim;
END_SAVES

BEGIN_COPYS(UIListArrow)
    COPY_SUPERCLASS(UIListWidget)
    CREATE_COPY_AS(UIListArrow, a);
    MILO_ASSERT(a, 0x5C);
    COPY_MEMBER_FROM(a, mMesh)
    COPY_MEMBER_FROM(a, mPosition)
    COPY_MEMBER_FROM(a, mShowOnlyScroll)
    COPY_MEMBER_FROM(a, mOnHighlight)
    COPY_MEMBER_FROM(a, mScrollAnim)
END_COPYS

BEGIN_LOADS(UIListArrow)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    UIListWidget::Load(bs);
    int dump;
    bool tmp;
    bs >> mMesh >> dump >> mShowOnlyScroll >> tmp;
    mOnHighlight = tmp;
    mPosition = (UIListArrowPosition)dump;
    if (d.rev > 0)
        bs >> mScrollAnim;
END_LOADS

void UIListArrow::Draw(
    const UIListWidgetDrawState &,
    const UIListState &,
    const Transform &,
    UIComponent::State,
    Box *,
    DrawCommand
) {}

void UIListArrow::StartScroll(int i, bool) {
    if (mScrollAnim == nullptr)
        return;
    if (i < 0) {
        if (!mPosition)
            goto a;
    }
    if (0 >= i)
        return;
    if (mPosition != 1)
        return;
a:
    mScrollAnim->Animate(0, false, 0, 0, kEaseLinear, 0, 0);
}
