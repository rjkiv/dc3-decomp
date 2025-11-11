#pragma once
#include "obj/Object.h"
#include "os/Platform.h"
#include "synth/SampleData.h"
#include "utl/MemMgr.h"

class SampleInst;

class SynthSample : public Hmx::Object {
public:
    enum SyncType {
        sync0,
        sync1,
        sync2,
        sync3
    };
    // Hmx::Object
    virtual ~SynthSample();
    OBJ_CLASSNAME(SynthSample);
    OBJ_SET_TYPE(SynthSample);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // SynthSample
    virtual SampleInst *NewInst(bool, int, int) { return nullptr; }
    virtual float LengthMs() const { return 0; }

    OBJ_MEM_OVERLOAD(0x18);

    int GetNumChannels() const;
    int NumMarkers() const;
    int GetPlatformSize(Platform);
    std::vector<SampleMarker> &AccessMarkers();
    void RegisterChild(SampleInst *);
    void UnregisterChild(SampleInst *);

    static void Init();
    static void Disable();

protected:
    SynthSample();

    virtual void Sync(SyncType);

    static SynthSample *sLoading;
    static FileLoader *sLoader;

    FilePath mFile; // 0x2c
    SampleData mSampleData; // 0x34
    std::list<SampleInst *> mSampleInsts; // 0x50
};
