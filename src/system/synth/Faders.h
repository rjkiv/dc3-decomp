#pragma once
#include "math/Easing.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "synth/Pollable.h"
#include "utl/BinStream.h"
#include <set>

class FaderGroup;

/** "A fader controls the volume level of one or more sound effects." */
class Fader : public Hmx::Object, public SynthPollable {
    friend class FaderGroup;

public:
    // Hmx::Object
    virtual ~Fader();
    OBJ_CLASSNAME(SynthFader);
    OBJ_SET_TYPE(SynthFader);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // SynthPollable
    virtual const char *GetSoundDisplayName();
    virtual void SynthPoll();

    float GetMLevelTarget() const { return mLevelTarget; }
    float GetPan() const { return mPan; }
    float GetTranspose() const { return mTranspose; }
    Symbol LocalName() const { return mLocalName; }
    void AddClient(FaderGroup *group) { mClients.insert(group); }
    void RemoveClient(FaderGroup *group) { mClients.erase(group); }
    void SetVolume(float);
    void SetPan(float);
    void SetTranspose(float);
    void DoFade(float);
    void DoFade(float, float);
    void AddDuckedVolume(float);
    void RemoveDuckedVolume(float);
    float DuckedValue() const { return GetDuckedVolume() + mLevel; }

    NEW_OBJ(Fader)

private:
    void UpdateValue(float, float, float);
    float GetDuckedVolume() const;

protected:
    Fader();

    /** "volume level in dB". Ranges from -96 to 6. */
    float mLevel; // 0x38
    EaseType mLevelEaseType; // 0x3c
    float mLevelEaseParam; // 0x40
    EaseFunc *mLevelEaseFunc; // 0x44
    float unk48;
    float mLevelTarget; // 0x4c
    float mPan; // 0x50
    EaseType mPanEaseType; // 0x54
    float mPanEaseParam; // 0x58
    EaseFunc *mPanEaseFunc; // 0x5c
    float unk60;
    float mPanTarget; // 0x64
    float mTranspose; // 0x68
    EaseType mTransposeEaseType; // 0x6c
    float mTransposeEaseParam; // 0x70
    EaseFunc *mTransposeEaseFunc; // 0x74
    float unk78;
    float mTransposeTarget; // 0x7c
    Symbol mLocalName; // 0x80
    std::set<FaderGroup *> mClients; // 0x84
    Timer mTimer; // 0xa0
    float unkd0; // 0xd0
    std::list<float> unkd4; // 0xd4
};

class FaderGroup {
public:
    FaderGroup(Hmx::Object *);
    ~FaderGroup();

    void SetDirty();
    void ClearDirty();
    float GetVolume() const;
    float GetPan() const;
    float GetTranspose() const;
    Fader *AddLocal(Symbol);
    Fader *FindLocal(Symbol, bool);
    void GetVal(float &, float &, float &) const;
    void Add(Fader *);
    void Remove(Fader *);
    void Save(BinStream &);
    void Load(BinStream &);

private:
    ObjPtrList<Fader> mFaders; // 0x0
    bool mDirty; // 0x14
};

bool PropSync(FaderGroup &, DataNode &, DataArray *, int, PropOp);
