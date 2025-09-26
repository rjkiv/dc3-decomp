#pragma once
#include "ui/UIList.h"
#include "utl/MemMgr.h"

class HamList : public UIList {
public:
    // Hmx::Object
    virtual ~HamList() {}
    OBJ_CLASSNAME(HamList);
    OBJ_SET_TYPE(HamList);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    OBJ_MEM_OVERLOAD(0x10)
    NEW_OBJ(HamList)
    static void Init();

protected:
    HamList();
};
