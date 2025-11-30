#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"

class FxSendBitCrush : public FxSend {
public:
    virtual ~FxSendBitCrush();
    OBJ_CLASSNAME(FxSendBitCrush);
    OBJ_SET_TYPE(FxSendBitCrush);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0xF);
    NEW_OBJ(FxSendBitCrush)

protected:
    FxSendBitCrush();

    float mAmount; // 0x60
};
