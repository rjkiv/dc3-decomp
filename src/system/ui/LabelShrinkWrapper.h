#pragma once
#include "obj/Object.h"
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

    NEW_OBJ(LabelShrinkWrapper)
    OBJ_MEM_OVERLOAD(0x14)

    static void Init();

protected:
    LabelShrinkWrapper();
    void Update();
    void UpdateAndDrawWrapper();

    ResourceDirPtr<RndDir> mResourceDir; // 0x44
    ObjPtr<UILabel> m_pLabel; // 0x5c
    bool m_pShow; // 0x70
    float mLeftBorder; // 0x74
    float mRightBorder; // 0x78
    float mTopBorder; // 0x7c
    float mBottomBorder; // 0x80
    RndMesh *m_pTopLeftBone; // 0x84
    RndMesh *m_pTopRightBone; // 0x88
    RndMesh *m_pBottomLeftBone; // 0x8c
    RndMesh *m_pBottomRightBone; // 0x90
};
