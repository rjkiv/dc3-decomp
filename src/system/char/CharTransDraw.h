#pragma once
#include "char/Character.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

/** "Defers drawing translucent parts of characters until this object is drawn" */
class CharTransDraw : public RndDrawable {
public:
    // Hmx::Object
    virtual ~CharTransDraw();
    OBJ_CLASSNAME(CharTransDraw)
    OBJ_SET_TYPE(CharTransDraw)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndDrawable
    virtual void DrawShowing();

    void SetDrawModes(Character::DrawMode);

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(CharTransDraw);

protected:
    CharTransDraw();

    /** "The Characters whose translucent bits we will draw" */
    ObjPtrList<Character> mChars; // 0x40
    /** "True if the transparent pieces should be drawn
        even if the character is not showing" */
    bool mForceDraw; // 0x54
};
