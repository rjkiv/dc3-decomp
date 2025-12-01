#pragma once
#include "obj/Object.h"
#include "ui/UIListWidget.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"

/**
 * @brief Highlighter object for UILists.
 * Original _objects description:
 * "Highlight widget for use with UIList"
 */
class UIListHighlight : public UIListWidget {
public:
    OBJ_CLASSNAME(UIListHighlight);
    OBJ_SET_TYPE(UIListHighlight);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Draw(
        const UIListWidgetDrawState &,
        const UIListState &,
        const Transform &,
        UIComponent::State,
        Box *,
        DrawCommand
    );

    NEW_OBJ(UIListHighlight)
    OBJ_MEM_OVERLOAD(0x11)

protected:
    UIListHighlight();

    /** "arrow mesh to draw/transform" */
    ObjPtr<RndMesh> mMesh; // 0x5c
};
