#include "synth/AudioDucker.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "utl/BinStream.h"

void AudioDuckerGroup::Duck() {
    for (int i = 0; i < mDuckers.size(); i++) {
        AudioDucker &cur = mDuckers[i];
        if (cur.mFader) {
            cur.mFader->AddDuckedVolume(cur.mLevel);
        }
    }
}

void AudioDuckerGroup::Unduck() {
    for (int i = 0; i < mDuckers.size(); i++) {
        AudioDucker &cur = mDuckers[i];
        if (cur.mFader) {
            cur.mFader->RemoveDuckedVolume(cur.mLevel);
        }
    }
}

void AudioDucker::Save(BinStream &bs) const {
    bs << mFader;
    bs << mLevel;
}

BEGIN_HANDLERS(AudioDuckerTrigger)
    HANDLE_ACTION(duck, mDuckerGroup.Duck())
    HANDLE_ACTION(unduck, mDuckerGroup.Unduck())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

AudioDucker::AudioDucker(Hmx::Object *owner) : mFader(owner), mLevel(0) {}

BEGIN_CUSTOM_PROPSYNC(AudioDucker)
    SYNC_PROP(fader, o.mFader)
    SYNC_PROP(level, o.mLevel)
END_CUSTOM_PROPSYNC

void AudioDucker::Load(BinStream &bs) {
    bs >> mFader;
    bs >> mLevel;
}

void AudioDuckerGroup::Save(BinStream &bs) { bs << mDuckers; }

BEGIN_SAVES(AudioDuckerTrigger)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    mDuckerGroup.Save(bs);
END_SAVES

AudioDuckerGroup::AudioDuckerGroup(Hmx::Object *owner) : mDuckers(owner) {}
AudioDuckerGroup::~AudioDuckerGroup() {}

AudioDuckerTrigger::AudioDuckerTrigger() : mDuckerGroup(this) {}

bool PropSync(AudioDuckerGroup &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op) {
    return PropSync(o.mDuckers, _val, _prop, _i, _op);
}

BEGIN_PROPSYNCS(AudioDuckerTrigger)
    SYNC_PROP(duckers, mDuckerGroup)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void AudioDuckerGroup::Load(BinStream &bs) { bs >> mDuckers; }

INIT_REVS(1, 0)

BEGIN_LOADS(AudioDuckerTrigger)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    mDuckerGroup.Load(bs);
END_LOADS

BEGIN_COPYS(AudioDuckerTrigger)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(AudioDuckerTrigger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDuckerGroup)
    END_COPYING_MEMBERS
END_COPYS
