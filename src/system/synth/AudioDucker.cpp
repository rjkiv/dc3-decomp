#include "synth/AudioDucker.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "utl/BinStream.h"

#pragma region AudioDucker

AudioDucker::AudioDucker(Hmx::Object *owner) : mFader(owner), mLevel(0) {}

void AudioDucker::Save(BinStream &bs) const {
    bs << mFader;
    bs << mLevel;
}

void AudioDucker::Load(BinStream &bs) {
    bs >> mFader;
    bs >> mLevel;
}

#pragma endregion
#pragma region AudioDuckerGroup

AudioDuckerGroup::AudioDuckerGroup(Hmx::Object *owner) : mDuckers(owner) {}
AudioDuckerGroup::~AudioDuckerGroup() {}

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

void AudioDuckerGroup::Save(BinStream &bs) { bs << mDuckers; }
void AudioDuckerGroup::Load(BinStream &bs) { bs >> mDuckers; }

void AudioDuckerGroup::Add(Fader *fader, float f2) {
    for (int i = 0; i < mDuckers.size(); i++) {
        if (mDuckers[i].mFader == fader) {
            mDuckers[i].mLevel = f2;
            return;
        }
    }
    AudioDucker ducker(mDuckers.Owner());
    ducker.mFader = fader;
    ducker.mLevel = f2;
    mDuckers.push_back(ducker);
}

void AudioDuckerGroup::Remove(Fader *fader) {
    FOREACH (it, mDuckers) {
        if (it->mFader == fader) {
            mDuckers.erase(it);
            return;
        }
    }
}

bool PropSync(AudioDuckerGroup &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op) {
    return PropSync(o.mDuckers, _val, _prop, _i, _op);
}

#pragma endregion
#pragma region AudioDuckerTrigger

AudioDuckerTrigger::AudioDuckerTrigger() : mDuckerGrp(this) {}
AudioDuckerTrigger::~AudioDuckerTrigger() {}

BEGIN_HANDLERS(AudioDuckerTrigger)
    HANDLE_ACTION(duck, mDuckerGrp.Duck())
    HANDLE_ACTION(unduck, mDuckerGrp.Unduck())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(AudioDucker)
    SYNC_PROP(fader, o.mFader)
    SYNC_PROP(level, o.mLevel)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(AudioDuckerTrigger)
    SYNC_PROP(duckers, mDuckerGrp)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(AudioDuckerTrigger)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    mDuckerGrp.Save(bs);
END_SAVES

BEGIN_COPYS(AudioDuckerTrigger)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(AudioDuckerTrigger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDuckerGrp)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(AudioDuckerTrigger)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    mDuckerGrp.Load(bs);
END_LOADS

#pragma endregion
