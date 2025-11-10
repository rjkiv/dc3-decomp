#pragma once
#include "rndobj/Dir.h"
#include "rndobj/Mesh.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"

/** "a mesh shrink wrapped to selected label" */
class LabelShrinkWrapper : public UIComponent {
public:
    // Hmx::Object
    virtual ~LabelShrinkWrapper();
    OBJ_CLASSNAME(LabelShrinkWrapper)
    OBJ_SET_TYPE(LabelShrinkWrapper)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetTypeDef(DataArray *);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    // UIComponent
    virtual void OldResourcePreload(BinStream &);

protected:
    LabelShrinkWrapper();

    ObjPtr<UILabel> m_pLabel; // 0x44

    RndMesh *m_pTopLeftBone;
    RndMesh *m_pTopRightBone;
    RndMesh *m_pBottomLeftBone;
    RndMesh *m_pBottomRightBone;
};
