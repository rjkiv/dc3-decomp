#include "ui/UIListMesh.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "ui/UIListSlot.h"
#include "utl/Loader.h"

#pragma region UIListMesh

UIListMesh::UIListMesh() : mMesh(this), mDefaultMat(this) {}

BEGIN_HANDLERS(UIListMesh)
    HANDLE_SUPERCLASS(UIListSlot)
END_HANDLERS

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

INIT_REVS(0, 0)

BEGIN_LOADS(UIListMesh)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(UIListSlot)
    d >> mMesh >> mDefaultMat;
END_LOADS

void UIListMesh::Draw(
    const UIListWidgetDrawState &drawstate,
    const UIListState &liststate,
    const Transform &tf,
    UIComponent::State compstate,
    Box *box,
    DrawCommand cmd
) {
    if (mMesh) {
        RndMat *mat = nullptr;
        float alpha = 1;
        if (TheLoadMgr.EditMode()) {
            mat = mMesh->Mat();
            if (mat) {
                alpha = mat->Alpha();
            }
        }
        Transform xfm = mMesh->LocalXfm();
        UIListSlot::Draw(drawstate, liststate, tf, compstate, box, cmd);
        mMesh->SetLocalXfm(xfm);
        if (TheLoadMgr.EditMode()) {
            mMesh->SetMat(mat);
            if (mat) {
                mat->SetAlpha(alpha);
            }
        }
    }
}

UIListSlotElement *UIListMesh::CreateElement(UIList *uilist) {
    MILO_ASSERT(mMesh, 0x5b);
    UIListSlotElement *element = new UIListMeshElement(this);
    return element;
}

RndTransformable *UIListMesh::RootTrans() { return mMesh; }

RndMat *UIListMesh::DefaultMat() const { return mDefaultMat; }

#pragma endregion
#pragma region UIListMeshElement

void UIListMeshElement::Draw(const Transform &tf, float f, UIColor *col, Box *box) {
    RndMesh *mesh = mListMesh->Mesh();
    MILO_ASSERT(mesh, 0x1B);
    mesh->SetWorldXfm(tf);
    if (box) {
        Box localbox(box->mMin, box->mMax);
        CalcBox(mesh, localbox);
        box->GrowToContain(localbox.mMin, false);
        box->GrowToContain(localbox.mMax, false);
    } else if (mMat) {
        float alpha = mMat->Alpha();
        mesh->SetMat(mMat);
        mMat->SetAlpha(alpha * f);
        if (col) {
            mMat->SetColor(col->GetColor());
        }
        mesh->DrawShowing();
        mMat->SetAlpha(alpha);
    }
}

#pragma endregion
