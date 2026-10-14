#pragma once
#include "FxSendMeterEffect.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

/** "Monitor for FxMeterEffect to feed back channel data" */
class MeterEffectMonitor : public RndPollable {
public:
    // Hmx::Object
    virtual ~MeterEffectMonitor();
    OBJ_CLASSNAME(MeterEffectMonitor);
    OBJ_SET_TYPE(MeterEffectMonitor);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x11)
    NEW_OBJ(MeterEffectMonitor)

protected:
    MeterEffectMonitor();

    /** "FxSendMeterEffect for this object to monitor" */
    ObjPtr<FxSendMeterEffect> mMeterEffect; // 0x8
    float mLastChannel1; // 0x1c
    float mLastChannel2; // 0x20
};
