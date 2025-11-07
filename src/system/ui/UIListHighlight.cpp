#include "ui/UIListHighlight.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIListWidget.h"

UIListHighlight::UIListHighlight() : mMesh(this) {}

BEGIN_PROPSYNCS(UIListHighlight)
    SYNC_PROP(mesh, mMesh)
    SYNC_SUPERCLASS(UIListWidget)
END_PROPSYNCS

BEGIN_SAVES(UIListHighlight)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIListWidget)
    bs << mMesh;
END_SAVES

BEGIN_COPYS(UIListHighlight)
    COPY_SUPERCLASS(UIListWidget)
    CREATE_COPY_AS(UIListHighlight, h)
    MILO_ASSERT(h, 0x38);
    COPY_MEMBER_FROM(h, mMesh)
END_COPYS

BEGIN_LOADS(UIListHighlight)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListWidget)
    bs >> mMesh;
END_LOADS

void UIListHighlight::Draw(
    const UIListWidgetDrawState &drawstate,
    const UIListState &liststate,
    const Transform &tf,
    UIComponent::State compstate,
    Box *box,
    DrawCommand cmd
) {}

BEGIN_HANDLERS(UIListHighlight)
    HANDLE_SUPERCLASS(UIListWidget)
END_HANDLERS
