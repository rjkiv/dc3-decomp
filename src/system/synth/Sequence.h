#pragma once
#include "obj/Object.h"
#include "synth/Pollable.h"
#include "synth/Faders.h"
#include "utl/PoolAlloc.h"

class SeqInst;

/** "A set of audio tasks" */
class Sequence : public Hmx::Object, public SynthPollable {
public:
    Sequence();
    virtual ~Sequence();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    virtual const char *GetSoundDisplayName() {
        return MakeString("Sequence: %s", Name());
    }
    virtual SeqInst *MakeInstImpl() = 0;
    virtual void SynthPoll();

    static void Init();

    float AvgVol() const { return mAvgVol; }
    float VolSpread() const { return mVolSpread; }
    float AvgTranspose() const { return mAvgTranspose; }
    float TransposeSpread() const { return mTransposeSpread; }
    float AvgPan() const { return mAvgPan; }
    float PanSpread() const { return mPanSpread; }
    FaderGroup &Faders() { return mFaders; }

    SeqInst *MakeInst();
    /** "Play the sequence" */
    SeqInst *Play(float, float, float);
    /** "Stop all instances of this sequence" */
    void Stop(bool);

protected:
    void OnTriggerSound(int);
    DataNode OnPlay(DataArray *);

    ObjPtrList<SeqInst> mInsts; // 0x38
    /** "Average volume this sequence will be played at, in dB" */
    float mAvgVol; // 0x4c
    /** "Amount to vary the volume above and below the average, in dB" */
    float mVolSpread; // 0x50
    /** "Average transpose this sequence will be played at, in semitones" */
    float mAvgTranspose; // 0x54
    /** "Amount to vary the transpose, in semitones" */
    float mTransposeSpread; // 0x58
    /** "Average pan to apply to this sequence (-1 - 1)" */
    float mAvgPan; // 0x5c
    /** "Amount to vary the pan" */
    float mPanSpread; // 0x60
    FaderGroup mFaders; // 0x64
    /** "If false, this sequence will play to its end and can't be stopped prematurely"
     */
    bool mCanStop; // 0x7c
};

/** "A Sequence type which just waits a specified duration, generating
 *  no sound.  Useful for tweaking the timing of other events." */
class WaitSeq : public Sequence {
public:
    WaitSeq();
    OBJ_CLASSNAME(WaitSeq);
    OBJ_SET_TYPE(WaitSeq);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual SeqInst *MakeInstImpl();

    float AvgWaitSecs() const { return mAvgWaitSecs; }
    float WaitSpread() const { return mWaitSpread; }

    NEW_OBJ(WaitSeq)
    static void Init() { REGISTER_OBJ_FACTORY(WaitSeq) }

protected:
    /** "Average wait time, in seconds" */
    float mAvgWaitSecs; // 0x80
    /** "Amount to vary the wait time, in seconds" */
    float mWaitSpread; // 0x84
};

/** "A sequence which plays other sequences.  Abstract base class." */
class GroupSeq : public Sequence {
public:
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    ObjPtrList<Sequence> &Children() { return mChildren; }

protected:
    GroupSeq();

    /** "The children of this sequence" */
    ObjPtrList<Sequence> mChildren; // 0x80
};

/** "Plays one or more of its child sequences, selected at random." */
class RandomGroupSeq : public GroupSeq {
public:
    RandomGroupSeq();
    OBJ_CLASSNAME(RandomGroupSeq);
    OBJ_SET_TYPE(RandomGroupSeq);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual SeqInst *MakeInstImpl();

    int NextIndex();
    void PickNextIndex();
    void ForceNextIndex(int);
    int GetNumSimul() { return mNumSimul; }
    bool AllowRepeats() const { return mAllowRepeats; }
    void AddToPlayedHistory(int idx);
    bool InPlayedHistory(int idx) const {
        std::list<int>::const_reverse_iterator it;
        for (it = mPlayHistory.rbegin(); it != mPlayHistory.rend(); it++) {
            if (*it == idx)
                return true;
        }
        return false;
    }

    NEW_OBJ(RandomGroupSeq)
    static void Init() { REGISTER_OBJ_FACTORY(RandomGroupSeq) }
    static void ForceSerialSequences(bool);
    static bool UsingSerialSequences();

protected:
    /** "Number of children to play simultaneously" */
    int mNumSimul; // 0x94
    /** "If false, you will never hear the same sequence again until all have played (only
     * if num_simul is 1)" */
    bool mAllowRepeats; // 0x98
    int mNextIndex; // 0x9c
    int mForceChooseIndex; // 0xa0
    std::list<int> mPlayHistory; // 0xa4
};

/** "Plays all of its child sequences at random intervals" */
class RandomIntervalGroupSeq : public GroupSeq {
public:
    RandomIntervalGroupSeq();
    OBJ_CLASSNAME(RandomIntervalGroupSeq);
    OBJ_SET_TYPE(RandomIntervalGroupSeq);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual SeqInst *MakeInstImpl();

    NEW_OBJ(RandomIntervalGroupSeq)
    static void Init() { REGISTER_OBJ_FACTORY(RandomIntervalGroupSeq) }

protected:
    /** "the number of seconds on average we wait to play a child cue again" */
    float mAvgIntervalSecs; // 0x94
    /** "We randomly deviate + or - this many seconds from the average when picking our
     * wait interval" */
    float mIntervalSpread; // 0x98
    /** "the maximum number of sounds we allow at one time" */
    int mMaxSimultaneous; // 0x9c
};

/** "Plays its child sequences in order, waiting for each to stop
 *  before moving on to the next." */
class SerialGroupSeq : public GroupSeq {
public:
    SerialGroupSeq() {}
    OBJ_CLASSNAME(SerialGroupSeq);
    OBJ_SET_TYPE(SerialGroupSeq);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);
    virtual SeqInst *MakeInstImpl();

    NEW_OBJ(SerialGroupSeq)
    static void Init() { REGISTER_OBJ_FACTORY(SerialGroupSeq) }
};

/** "Plays all of its child sequences at the same time." */
class ParallelGroupSeq : public GroupSeq {
public:
    ParallelGroupSeq() {}
    OBJ_CLASSNAME(ParallelGroupSeq);
    OBJ_SET_TYPE(ParallelGroupSeq);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);
    virtual SeqInst *MakeInstImpl();

    NEW_OBJ(ParallelGroupSeq)
    static void Init() { REGISTER_OBJ_FACTORY(ParallelGroupSeq) }
};

class SfxSeq : public SerialGroupSeq {
public:
    SfxSeq();
    virtual ~SfxSeq() {}
    OBJ_CLASSNAME(SfxSeq);
    OBJ_SET_TYPE(SfxSeq);
    virtual void Save(BinStream &);
    virtual void Load(BinStream &);

    NEW_OBJ(SfxSeq)
    static void Init() { REGISTER_OBJ_FACTORY(SfxSeq) }
};

class SeqInst : public Hmx::Object {
public:
    SeqInst(Sequence *);
    virtual ~SeqInst();
    virtual void Stop() = 0;
    virtual bool IsRunning() = 0;
    virtual void UpdateVolume() = 0;
    virtual void SetPan(float) = 0;
    virtual void SetTranspose(float) = 0;
    virtual void Poll() {}
    virtual void StartImpl() = 0;

    POOL_OVERLOAD(SeqInst, 0x11f)

    void Start();
    void SetVolume(float);
    bool Started() const { return mStarted; }

protected:
    Sequence *mOwner; // 0x2c
    float mRandVol; // 0x30
    float mRandPan; // 0x34
    float mRandTp; // 0x38
    float mVolume; // 0x3c
    bool mStarted; // 0x40
};

#include "Sequence_p.h"
