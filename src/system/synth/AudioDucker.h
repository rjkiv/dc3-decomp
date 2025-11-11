#pragma once
#include "obj/Object.h"
#include "synth/Faders.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class AudioDucker {
    friend bool
    PropSync(AudioDucker &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);
    friend class AudioDuckerGroup;

public:
    AudioDucker(Hmx::Object *);
    void Save(BinStream &) const;
    void Load(BinStream &);

private:
    /** "Fader that will be ducked." */
    ObjPtr<Fader> mFader; // 0x0
    /** "Ducking volume level in dB". Ranges from -96 to 6. */
    float mLevel; // 0x14
};

inline BinStream &operator<<(BinStream &bs, const AudioDucker &d) {
    d.Save(bs);
    return bs;
}

inline BinStream &operator>>(BinStream &bs, AudioDucker &d) {
    d.Load(bs);
    return bs;
}

class AudioDuckerGroup {
    friend bool
    PropSync(AudioDuckerGroup &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);

public:
    AudioDuckerGroup(Hmx::Object *owner);
    ~AudioDuckerGroup() {}
    void Duck();
    void Unduck();
    void Save(BinStream &);
    void Load(BinStream &);
    void Add(Fader *, float);
    void Remove(Fader *);

protected:
    ObjVector<AudioDucker> mDuckers; // 0x0
};

/** "This object helps triggering audio duckers that are not attached to anything." */
class AudioDuckerTrigger : public Hmx::Object {
public:
    virtual ~AudioDuckerTrigger();
    OBJ_CLASSNAME(AudioDuckerTrigger);
    OBJ_SET_TYPE(AudioDuckerTrigger);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x3F);
    NEW_OBJ(AudioDuckerTrigger)

protected:
    AudioDuckerTrigger();

    AudioDuckerGroup mDuckerGroup; // 0x2c
};
