#pragma once
#include "obj/Dir.h"
#include "rndobj/Trans.h"

/**
 * @brief A RndTransformable but within a proxy.
 * Original _objects description:
 * "Stand-in for a RndTransformable inside of a proxy, so you can use it"
 */
class RndTransProxy : public RndTransformable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(TransProxy);
    OBJ_SET_TYPE(TransProxy);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PostSave(BinStream &);

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(RndTransProxy)
    REGISTER_OBJ(RndTransProxy)

    void SetProxy(ObjectDir *);
    void SetPart(Symbol);
    void Sync();

protected:
    RndTransProxy();

    /** "Proxy object this will look into." */
    ObjPtr<ObjectDir> mProxy; // 0xc0
    /** "The part inside it" */
    Symbol mPart; // 0xd4
};
