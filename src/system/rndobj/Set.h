#pragma once
#include "obj/Object.h"
#include "obj/Object.h"
#include <vector>

/**
 * @brief: A group of objects that send messages.
 * Original _objects description:
 * "A group of objects to propagate animation and messages"
 */
class RndSet : public Hmx::Object {
public:
    OBJ_CLASSNAME(Set);
    OBJ_SET_TYPE(Set);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetTypeDef(DataArray *);

protected:
    RndSet();

    /** Determine whether or not the supplied object is allowed to be part of this RndSet.
     */
    bool AllowedObject(Hmx::Object *o);
    /** Get a DataArray of all allowed Objects in this RndSet's dir.
     * @returns A DataNode housing the aforementioned DataArray.
     * Example usage: {$this allowed_objects}
     */
    DataNode OnAllowedObjects(DataArray *);

    /** The list of allowed objects. */
    ObjPtrList<Hmx::Object> mObjects; // 0x2c
    /** The collection of properties that allowed objects must have. */
    std::vector<Symbol> mProps; // 0x40
};
