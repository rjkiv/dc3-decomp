#pragma once
#include "char/Character.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Reflects all drawables in draws." */
class WorldReflection : public RndDrawable, public RndTransformable {
public:
    // Hmx::Object
    virtual ~WorldReflection();
    OBJ_CLASSNAME(WorldReflection);
    OBJ_SET_TYPE(WorldReflection);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndHighlightable
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(WorldReflection)

protected:
    WorldReflection();

    void DoHide();
    void UnHide();
    void DoLOD(int);

    /** "things to draw in the reflection, in this order" */
    ObjPtrList<RndDrawable> mDraws; // 0x100
    /** "Set LOD to 1 on these reflected characters" */
    ObjPtrList<Character> mLodChars; // 0x114
    /** "How far to stretch vertically" */
    float mVerticalStretch; // 0x128
    std::list<RndMat *> unk12c; // 0x12c
    RndCam *unk134; // 0x134
    bool unk138; // 0x138
    /** "List of objects to hide in the reflection,
        shows them when reflection has finished drawing." */
    ObjPtrList<RndDrawable> mHideList; // 0x13c
    /** "List of objects to show in the reflection,
        hides them when reflection has finished drawing." */
    ObjPtrList<RndDrawable> mShowList; // 0x15c
    ObjPtrList<RndDrawable> unk164; // 0x164
    ObjPtrList<RndDrawable> unk178; // 0x178
};
