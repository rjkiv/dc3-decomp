#pragma once
#include "hamobj/HamLabel.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "utl/MemMgr.h"

/** "Stars Display" */
class StarsDisplay : public UIComponent, public UIListCustomTemplate {
public:
    // Hmx::Object
    virtual ~StarsDisplay();
    OBJ_CLASSNAME(StarsDisplay);
    OBJ_SET_TYPE(StarsDisplay);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll() {}
    // UIListCustomTemplate
    virtual void SetAlphaColor(float, UIColor *);
    virtual void GrowBoundingBox(Box &) const {}

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(StarsDisplay)
    static void Init();

protected:
    StarsDisplay();

    virtual void OldResourcePreload(BinStream &);

    void Update();
    void SetStars(int);

    ResourceDirPtr<ObjectDir> mResourceDir; // 0x48
    HamLabel *mStarsLabel; // 0x60
    HamLabel *mDiffLabel; // 0x64
    HamLabel *mNoFlashcardsLabel; // 0x68
    bool mShowUnplayedSong; // 0x6c
    float mAlpha; // 0x70
};
