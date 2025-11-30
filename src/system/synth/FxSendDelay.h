#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/MemMgr.h"

/** "A delay effect." */
class FxSendDelay : public FxSend {
public:
    virtual ~FxSendDelay();
    OBJ_CLASSNAME(FxSendDelay);
    OBJ_SET_TYPE(FxSendDelay);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0xF);
    NEW_OBJ(FxSendDelay)

protected:
    FxSendDelay();

    float mDelayTime; // 0x60
    float mGain; // 0x64
    /** "Depth of ping pong effect (percent)". Ranges from 0 to 100. */
    float mPingPongPct; // 0x68
    /** "Sync delay to song tempo?" */
    bool mTempoSync; // 0x6c
    /** "Note value of delay". Sync type options: (sixteenth eighth dotted_eighth quarter
     * dotted_quarter half whole) */
    Symbol mSyncType; // 0x70
    /** "Tempo for delay; can be driven by game code". Ranges from 20 to 300. */
    float mTempo; // 0x74
};
