#pragma once
#include "flow/FlowLabelProvider.h"
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "synth/Sound.h"

/** "Plays a sound cue" */
class FlowSound : public FlowNode, public FlowLabelProvider {
public:
    // Hmx::Object
    virtual ~FlowSound();
    OBJ_CLASSNAME(FlowSound)
    OBJ_SET_TYPE(FlowSound)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual bool IsRunning();
    virtual void UpdateIntensity();

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(FlowSound)

protected:
    FlowSound();

    void OnSoundSelected();
    void OnMarkerEvent(Symbol);

    /** "do not wait for sound to finish before finishing flow execution" */
    bool mImmediateRelease; // 0x5c
    /** "How should we handle stop requests?" */
    StopMode mStopMode; // 0x60
    bool unk64;
    int unk68;
    bool unk6c;
    /** "The sound file to play" */
    FlowPtr<Sound> mSound; // 0x70
    /** "Volume of the sound, in Db" */
    float mVolume; // 0x90
    /** "pitch adjustment of the sound in semitones" */
    float mTranspose; // 0x94
    /** "Pan of the sound, -4 to +4" */
    float mPan; // 0x98
    bool unk9c;
    /** "If true, we stop all instances of this sound from playing" */
    bool mForceStop; // 0x9d
    /** "Do we pass on running intensity to volume?" */
    bool mUseIntensity; // 0x9e
    float unka0;
};
