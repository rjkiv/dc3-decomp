#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

// courtesy of RB2
struct LevelData {
    LevelData(const char *name)
        : mRMS(0), mPeak(0), mPeakHold(0), mPeakAge(0), mName(name) {}
    float mRMS; // offset 0x0, size 0x4
    float mPeak; // offset 0x4, size 0x4
    float mPeakHold; // offset 0x8, size 0x4
    int mPeakAge; // offset 0xC, size 0x4
    class String mName; // offset 0x10, size 0x8
};

enum SendChannels {
    kSendAll = 0,
    kSendCenter = 1,
    kSendStereo = 2,
    kSendAllXMix = 3
};

/** "Base class for all sound FX processors" */
class FxSend : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~FxSend() {}
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(FxSend);
    OBJ_SET_TYPE(FxSend);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // FxSend
    virtual void SetNextSend(FxSend *);
    virtual void Recreate(std::vector<FxSend *> &) {}
    virtual void RebuildChain();
    virtual void BuildChainVector(std::vector<FxSend *> &);
    virtual bool CanPushParameters() { return true; }

    OBJ_MEM_OVERLOAD(0x19);

    FxSend *NextSend() const { return mNextSend; }
    int Stage() const { return mStage; }
    void SetChannels(SendChannels);
    void EnableUpdates(bool);
    /** "Attach microphone to this send, for testing" */
    void TestWithMic();
    void SetStage(int);
    bool UpdatesEnabled() const { return mEnableUpdates; }
    SendChannels GetChannels() const { return mChannels; }
    bool ReverbEnabled() const { return mReverbEnable; }

protected:
    FxSend();

    virtual void UpdateMix() {}
    virtual void OnParametersChanged() {}

    bool CheckChain(FxSend *, int);

    /** "The next effect in the chain" */
    ObjOwnerPtr<FxSend> mNextSend; // 0x2c
    /** "The relative order that this send is processed compared to other sends." Ranges
     * from 0 to 9. */
    int mStage; // 0x40
    /** "Bypass the effect and stop it from processing" */
    bool mBypass; // 0x44
    /** "Gain applied to dry signal (dB)". Ranges from -96.0 to 20.0. */
    float mDryGain; // 0x48
    /** "Gain applied to effect output (dB)" Ranges from -96.0 to 20.0. */
    float mWetGain; // 0x4c
    /** "Gain applied to effect input (dB)" Ranges from -96.0 to 20.0. */
    float mInputGain; // 0x50
    /** "Reverb send for this effect". Ranges from -96.0 to 20.0. */
    float mReverbMixDb; // 0x54
    /** "Enable reverb send" */
    bool mReverbEnable; // 0x58
    bool mEnableUpdates; // 0x59
    /** "Which channels the FX applies to" */
    SendChannels mChannels; // 0x5c
};
