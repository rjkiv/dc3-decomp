#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/Str.h"
#include <vector>

/** "meter effect" */
class FxSendMeterEffect : public FxSend {
public:
    OBJ_CLASSNAME(FxSendMeterEffect);
    OBJ_SET_TYPE(FxSendMeterEffect);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x11);
    NEW_OBJ(FxSendMeterEffect)

    float ChannelData(int);

protected:
    FxSendMeterEffect();

    bool mResetPeaks; // 0x60
    std::vector<LevelData> mChannels; // 0x64
};
