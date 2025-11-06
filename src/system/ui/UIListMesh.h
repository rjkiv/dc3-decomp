#pragma once
#include "ui/UIListProvider.h"
#include "ui/UIListSlot.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"

/** "Custom slot for use with UIList" */
class UIListMesh : public UIListSlot {
public:
    OBJ_CLASSNAME(UIListMesh)
    OBJ_SET_TYPE(UIListMesh)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void Draw(
        const UIListWidgetDrawState &,
        const UIListState &,
        const Transform &,
        UIComponent::State,
        Box *,
        DrawCommand
    );

    RndMat *DefaultMat() const;
    RndMesh *Mesh() const { return mMesh; }

protected:
    virtual UIListSlotElement *CreateElement(UIList *);
    virtual RndTransformable *RootTrans();
    UIListMesh();

    /** "mesh to draw/transform" */
    ObjPtr<RndMesh> mMesh; // 0x78
    /** "default material" */
    ObjPtr<RndMat> mDefaultMat; // 0x8c
};

class UIListMeshElement : public UIListSlotElement {
public:
    UIListMeshElement(UIListMesh *lm) : mListMesh(lm), mMat(0) {}
    virtual ~UIListMeshElement() {}
    virtual void Fill(const UIListProvider &prov, int i, int j) {
        mMat = prov.Mat(i, j, mListMesh);
    }
    virtual void Draw(const Transform &, float, UIColor *, Box *);

    UIListMesh *mListMesh;
    RndMat *mMat;
};
