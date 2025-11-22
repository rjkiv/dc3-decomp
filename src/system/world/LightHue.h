#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "math/Key.h"
#include "utl/MemMgr.h"

class LightHue : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~LightHue();
    OBJ_CLASSNAME(LightHue)
    OBJ_SET_TYPE(LightHue)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(LightHue)

    void TranslateColor(Hmx::Color const &, Hmx::Color &);

private:
    void Sync();
    DataNode OnSaveDefault(DataArray *);

protected:
    LightHue();

    FileLoader *mLoader; // 0x2c
    FilePath mPath; // 0x30
    Keys<Vector3, Vector3> mKeys; // 0x38
};
