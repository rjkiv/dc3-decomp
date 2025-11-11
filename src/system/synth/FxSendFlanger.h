#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"

/** "A flanger effect." */
class FxSendFlanger : public FxSend {
public:
    OBJ_CLASSNAME(FxSendFlanger);
    OBJ_SET_TYPE(FxSendFlanger);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x10);
    NEW_OBJ(FxSendFlanger)

protected:
    FxSendFlanger();

    /** "Maximum delay time in milliseconds". Ranges from 0 to 10. */
    float mDelayMs; // 0x60
    /** "Rate at which delay is modulated (Hz)". Ranges from 0 to 10. */
    float mRate; // 0x64
    /** "Percent depth of delay modulation". Ranges from 0 to 100. */
    float mDepthPct; // 0x68
    /** "Percent of output that is fed back to input". Ranges from 0 to 100. */
    float mFeedbackPct; // 0x6c
    /** "LFO phase offset between channels (for wider stereo effect)". Ranges from 0 to
     * 100. */
    float mOffsetPct; // 0x70
    /** "Sync modulation to song tempo?" */
    bool mTempoSync; // 0x74
    /** "Note value to synchronize modulation to". Sync type options: (sixteenth eighth
     * dotted_eighth quarter dotted_quarter half whole) */
    Symbol mSyncType; // 0x78
    /** "Tempo for modulation; can be driven by game code". Ranges from 20 to 300. */
    float mTempo; // 0x7c
};
