#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"

/** "A compression/expansion effect." */
class FxSendCompress : public FxSend {
public:
    virtual ~FxSendCompress();
    OBJ_CLASSNAME(FxSendCompress);
    OBJ_SET_TYPE(FxSendCompress);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0xF);
    NEW_OBJ(FxSendCompress)

protected:
    FxSendCompress();

    /** "threshold (in dB) at which compression is applied". Ranges from -96 to 0 */
    float mThresholdDB; // 0x60
    /** "Compression factor - ratio of input level to output level". Ranges from 1 to 80
     */
    float mRatio; // 0x64
    /** "output level for a maxed signal, in dB". Ranges from -10 to 10. */
    float mOutputLevel; // 0x68
    /** "Attack time in seconds". Ranges from 1.0e-3 to 1. */
    float mAttack; // 0x6c
    /** "Release time in seconds". Ranges from 1.0e-3 to 2. */
    float mRelease; // 0x70
    /** "Expansion factor - ratio of input level to output level.  The expander uses the
     * same threshold as the compressor.". Ranges from 1 to 20. */
    float mExpRatio; // 0x74
    /** "Attack time in seconds". Ranges from 1.0e-3 to 2. */
    float mExpAttack; // 0x78
    /** "Release time in seconds". Ranges from 1.0e-3 to 1. */
    float mExpRelease; // 0x7c
    /** "threshold (in dB) at which gating is applied". Ranges from -96 to 0 */
    float mGateThresholdDB; // 0x80
};
