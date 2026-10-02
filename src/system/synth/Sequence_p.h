#pragma once
#include "synth/Sequence.h"
#include "utl/PoolAlloc.h"

// DO NOT try to include this header directly!
// include synth/Sequence.h instead

class WaitSeqInst : public SeqInst {
public:
    WaitSeqInst(WaitSeq *);
    virtual ~WaitSeqInst() {}
    virtual void Stop();
    virtual bool IsRunning();
    virtual void UpdateVolume() {}
    virtual void SetPan(float) {}
    virtual void SetTranspose(float) {}

    POOL_OVERLOAD(WaitSeqInst, 0x1A);

protected:
    virtual void StartImpl();

    float mWaitMs; // 0x34
    float mEndTime; // 0x38
};

class GroupSeqInst : public SeqInst {
public:
    GroupSeqInst(GroupSeq *, bool);
    virtual ~GroupSeqInst();
    virtual void UpdateVolume();
    virtual void SetPan(float);
    virtual void SetTranspose(float);
    virtual void Poll() {}

    POOL_OVERLOAD(GroupSeqInst, 0x34);

protected:
    ObjVector<ObjPtr<SeqInst> > mSeqs; // 0x34
};

class RandomGroupSeqInst : public GroupSeqInst {
public:
    RandomGroupSeqInst(RandomGroupSeq *);
    virtual ~RandomGroupSeqInst() {}
    virtual void Stop();
    virtual bool IsRunning();

    POOL_OVERLOAD(RandomGroupSeqInst, 0x78);

protected:
    virtual void Poll();
    virtual void StartImpl();

    int mNumSeqs; // 0x40
    ObjVector<ObjPtr<SeqInst> >::iterator mIt; // 0x44
};

class RandomIntervalGroupSeqInst : public GroupSeqInst {
public:
    RandomIntervalGroupSeqInst(RandomIntervalGroupSeq *);
    virtual ~RandomIntervalGroupSeqInst() {}
    virtual void Stop();
    virtual bool IsRunning();

    POOL_OVERLOAD(RandomIntervalGroupSeqInst, 0x90);

protected:
    virtual void Poll();
    virtual void StartImpl();
    void ComputeNextTime(int);

    int mMaxSimultaneous; // 0x40
    float mAvgIntervalSecs; // 0x44
    float mIntervalSpread; // 0x48
    std::vector<float> unk4c;
    bool mRunning; // 0x54
};

class SerialGroupSeqInst : public GroupSeqInst {
public:
    SerialGroupSeqInst(SerialGroupSeq *);
    virtual void Stop();
    virtual bool IsRunning();

    POOL_OVERLOAD(SerialGroupSeqInst, 0x4B);

protected:
    virtual void Poll();
    virtual void StartImpl();

    ObjVector<ObjPtr<SeqInst> >::iterator mIt; // 0x40
};

class ParallelGroupSeqInst : public GroupSeqInst {
public:
    ParallelGroupSeqInst(ParallelGroupSeq *);
    virtual ~ParallelGroupSeqInst() {}
    virtual void Stop();
    virtual bool IsRunning();

    POOL_OVERLOAD(ParallelGroupSeqInst, 0x61);

protected:
    virtual void Poll();
    virtual void StartImpl();

    ObjVector<ObjPtr<SeqInst> >::iterator mIt; // 0x40
};
