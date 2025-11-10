#include "ui/UIListMesh.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "ui/UIListSlot.h"
#include "utl/Loader.h"

#pragma region UIListMesh

UIListMesh::UIListMesh() : mMesh(this), mDefaultMat(this) {}

BEGIN_PROPSYNCS(UIListMesh)
    SYNC_PROP(mesh, mMesh)
    SYNC_PROP(default_mat, mDefaultMat)
    SYNC_SUPERCLASS(UIListSlot)
END_PROPSYNCS

BEGIN_SAVES(UIListMesh)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIListSlot)
    bs << mMesh << mDefaultMat;
END_SAVES

BEGIN_COPYS(UIListMesh)
    COPY_SUPERCLASS(UIListSlot)
    CREATE_COPY_AS(UIListMesh, m)
    MILO_ASSERT(m, 0x9F);
    COPY_MEMBER_FROM(m, mMesh)
    COPY_MEMBER_FROM(m, mDefaultMat)
END_COPYS

BEGIN_LOADS(UIListMesh)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListSlot)
    bs >> mMesh >> mDefaultMat;
END_LOADS

void UIListMesh::Draw(
    const UIListWidgetDrawState &drawstate,
    const UIListState &liststate,
    const Transform &tf,
    UIComponent::State compstate,
    Box *box,
    DrawCommand cmd
) {}

UIListSlotElement *UIListMesh::CreateElement(UIList *uilist) {
    MILO_ASSERT(mMesh, 0x5b);
    UIListSlotElement *element = new UIListMeshElement(this);
    return element;
}

RndTransformable *UIListMesh::RootTrans() { return mMesh; }

#pragma endregion UIListMesh
#pragma region UIListMeshElement

inline void
UIListMeshElement::Draw(const Transform &tf, float f, UIColor *col, Box *box) {}

#pragma endregion UIListMeshElement
