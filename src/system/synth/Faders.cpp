#include "synth/Faders.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

void FaderGroup::SetDirty() { mDirty = true; }
void FaderGroup::ClearDirty() { mDirty = false; }
bool FaderGroup::Dirty() { return mDirty; }

void Fader::Save(BinStream &bs) {
    if (!mLocalName.Null()) {
        MILO_FAIL("can't save local fader");
    }
    bs << 1;
    SAVE_SUPERCLASS(Hmx::Object);
    bs << mLevel;
    bs << mLevelEaseType;
    bs << mLevelEaseParam;
    bs << mLevelTarget;
    bs << mPan;
    bs << mPanEaseType;
    bs << mPanEaseParam;
    bs << mPanTarget;
    bs << mTranspose;
    bs << mTransposeEaseType;
    bs << mTransposeEaseParam;
    bs << mTransposeTarget;
}

void Fader::DoFade(float durationMs) {
    MILO_ASSERT(durationMs >= 0.0f, 0x5B);
    unkd0 = durationMs;
    unk48 = mLevel;
    unk60 = mPan;
    unk78 = mTranspose;
    mTimer.Stop();
    CancelPolling();
    StartPolling();
    mTimer.Restart();
    MILO_ASSERT(mTimer.Running(), 0x65);
}

float Fader::GetDuckedVolume() const {
    float vol = 0;
    for (std::list<float>::const_iterator it = unkd4.begin(); it != unkd4.end(); ++it) {
        vol = Min(vol, *it);
    }
    return vol;
}

float FaderGroup::GetPan() const {
    float pan = 0;
    for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end(); ++it) {
        pan += (*it)->GetPan();
    }
    return pan;
}

float FaderGroup::GetTranspose() const {
    float transpose = 0;
    for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end(); ++it) {
        transpose += (*it)->GetTranspose();
    }
    return transpose;
}

Fader *FaderGroup::FindLocal(Symbol name, bool fail) {
    if (!name.Null()) {
        for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end();
             ++it) {
            if ((*it)->LocalName() == name) {
                return *it;
            }
        }
    }
    if (fail) {
        MILO_FAIL("bad local fader: %s", name);
    }
    return nullptr;
}

void Fader::DoFade(float targetVol, float durationMs) {
    mLevelTarget = targetVol;
    DoFade(durationMs);
}

void Fader::UpdateValue(float vol, float pan, float transpose) {
    mLevel = vol;
    mPan = pan;
    mTranspose = transpose;
    for (std::set<FaderGroup *>::iterator it = mClients.begin(); it != mClients.end();
         ++it) {
        (*it)->SetDirty();
    }
}

void Fader::RemoveDuckedVolume(float f1) {
    std::list<float>::iterator it = unkd4.begin();
    for (; it != unkd4.end(); ++it) {
        if (*it == f1) {
            unkd4.erase(it);
            for (std::set<FaderGroup *>::iterator it = mClients.begin();
                 it != mClients.end();
                 ++it) {
                (*it)->SetDirty();
            }
            return;
        }
    }
}

float FaderGroup::GetVolume() const {
    float vol = 0;
    for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end(); ++it) {
        float duckedVol = 0;
        for (std::list<float>::const_iterator cit = (*it)->unkd4.begin();
             cit != (*it)->unkd4.end();
             ++cit) {
            duckedVol = Min(duckedVol, *cit);
        }
        vol += (*it)->mLevel + duckedVol;
    }
    return vol;
}

void FaderGroup::GetVal(float &vol, float &pan, float &transpose) const {
    vol = 0;
    pan = 0;
    transpose = 0;
    for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end(); ++it) {
        float val = 0;
        for (std::list<float>::const_iterator cit = (*it)->unkd4.begin();
             cit != (*it)->unkd4.end();
             ++cit) {
            val = Min(val, *cit);
        }
        vol += (*it)->mLevel + val;
        pan += (*it)->mPan;
        transpose = (*it)->mTranspose;
    }
}

void Fader::SetVolume(float vol) {
    mTimer.Stop();
    CancelPolling();
    UpdateValue(vol, mPan, mTranspose);
}

void Fader::SetPan(float pan) {
    mTimer.Stop();
    CancelPolling();
    UpdateValue(mLevel, pan, mTranspose);
}

void Fader::SetTranspose(float transpose) {
    mTimer.Stop();
    CancelPolling();
    UpdateValue(mLevel, mPan, transpose);
}

void Fader::AddDuckedVolume(float vol) {
    unkd4.push_back(vol);
    for (std::set<FaderGroup *>::iterator it = mClients.begin(); it != mClients.end();
         ++it) {
        (*it)->SetDirty();
    }
}

BEGIN_COPYS(Fader)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(Fader)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            SetVolume(c->mLevel);
            COPY_MEMBER(mLevelEaseType)
            mLevelEaseFunc = GetEaseFunction(mLevelEaseType);
            COPY_MEMBER(mLevelEaseParam)
            COPY_MEMBER(mLevelTarget)
            SetPan(c->mPan);
            COPY_MEMBER(mPanEaseType)
            mPanEaseFunc = GetEaseFunction(mPanEaseType);
            COPY_MEMBER(mPanEaseParam)
            COPY_MEMBER(mPanTarget)
            SetTranspose(c->mTranspose);
            COPY_MEMBER(mTransposeEaseType)
            mTransposeEaseFunc = GetEaseFunction(mTransposeEaseType);
            COPY_MEMBER(mTransposeEaseParam)
            COPY_MEMBER(mTransposeTarget)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_PROPSYNCS(Fader)
    SYNC_PROP_SET(level, mLevel, SetVolume(_val.Float()))
    SYNC_PROP_SET(
        level_ease_type, mLevelEaseType, mLevelEaseType = (EaseType)_val.Int();
        mLevelEaseFunc = GetEaseFunction(mLevelEaseType)
    )
    SYNC_PROP_SET(level_ease_param, mLevelEaseParam, mLevelEaseParam = _val.Float())
    SYNC_PROP_SET(level_target, mLevelTarget, mLevelTarget = _val.Float())
    SYNC_PROP_SET(pan, mPan, SetPan(_val.Float()))
    SYNC_PROP_SET(
        pan_ease_type, mPanEaseType, mPanEaseType = (EaseType)_val.Int();
        mPanEaseFunc = GetEaseFunction(mPanEaseType)
    )
    SYNC_PROP_SET(pan_ease_param, mPanEaseParam, mPanEaseParam = _val.Float())
    SYNC_PROP_SET(pan_target, mPanTarget, mPanTarget = _val.Float())
    SYNC_PROP_SET(transpose, mTranspose, SetTranspose(_val.Float()))
    SYNC_PROP_SET(
        transpose_ease_type,
        mTransposeEaseType,
        mTransposeEaseType = (EaseType)_val.Int();
        mTransposeEaseFunc = GetEaseFunction(mTransposeEaseType)
    )
    SYNC_PROP_SET(
        transpose_ease_param, mTransposeEaseParam, mTransposeEaseParam = _val.Float()
    )
    SYNC_PROP_SET(transpose_target, mTransposeTarget, mTransposeTarget = _val.Float())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_HANDLERS(Fader)
    HANDLE_ACTION(set_val, SetVolume(_msg->Float(2)))
    HANDLE_ACTION(fade, DoFade(_msg->Float(2), _msg->Float(3)))
    HANDLE_ACTION(do_fade, DoFade(_msg->Float(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

Fader::~Fader() {
    mTimer.Stop();
    CancelPolling();
}

FaderGroup::FaderGroup(Hmx::Object *owner) : mFaders(owner), mDirty(true) {}

FaderGroup::~FaderGroup() {
    while (!mFaders.empty()) {
        Fader *frontObj = mFaders.front();
        mFaders.pop_front();
        frontObj->RemoveClient(this);
        if (!frontObj->LocalName().Null()) {
            delete frontObj;
        }
    }
}

Fader::Fader()
    : mLevel(0), mLevelEaseParam(2), mLevelTarget(0), mPan(0), mPanEaseParam(1),
      mPanTarget(0), mTranspose(0), mTransposeEaseParam(1), mTransposeTarget(0),
      unkd0(0) {
    unkd4.clear();
    mLevelEaseType = kEasePolyOut;
    mLevelEaseFunc = gEaseFuncs[mLevelEaseType];
    mPanEaseType = kEaseLinear;
    mPanEaseFunc = gEaseFuncs[mPanEaseType];
    mTransposeEaseType = kEaseLinear;
    mTransposeEaseFunc = gEaseFuncs[mTransposeEaseType];
}
