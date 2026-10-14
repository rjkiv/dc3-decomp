#include "synth/Faders.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

#pragma region Fader

Fader::Fader()
    : mVolumeDb(0), mVolumeEaseParam(2), mTargetVolumeDb(0), mPan(0), mPanEaseParam(1),
      mTargetPan(0), mTranspose(0), mTransposeEaseParam(1), mTargetTranspose(0),
      mDurationMs(0) {
    mDuckedVolumes.clear();
    mVolumeEaseType = kEasePolyOut;
    mVolumeEaseFunc = GetEaseFunction(kEasePolyOut);
    mPanEaseType = kEaseLinear;
    mPanEaseFunc = GetEaseFunction(kEaseLinear);
    mTransposeEaseType = kEaseLinear;
    mTransposeEaseFunc = GetEaseFunction(kEaseLinear);
}

Fader::~Fader() { CancelFade(); }

void Fader::CancelFade() {
    mTimer.Stop();
    CancelPolling();
}

BEGIN_HANDLERS(Fader)
    HANDLE_ACTION(set_val, SetVolume(_msg->Float(2)))
    HANDLE_ACTION(fade, DoFade(_msg->Float(2), _msg->Float(3)))
    HANDLE_ACTION(do_fade, DoFade(_msg->Float(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Fader)
    SYNC_PROP_SET(level, mVolumeDb, SetVolume(_val.Float()))
    SYNC_PROP_SET(
        level_ease_type, mVolumeEaseType, mVolumeEaseType = (EaseType)_val.Int();
        mVolumeEaseFunc = GetEaseFunction(mVolumeEaseType)
    )
    SYNC_PROP_SET(level_ease_param, mVolumeEaseParam, mVolumeEaseParam = _val.Float())
    SYNC_PROP_SET(level_target, mTargetVolumeDb, mTargetVolumeDb = _val.Float())
    SYNC_PROP_SET(pan, mPan, SetPan(_val.Float()))
    SYNC_PROP_SET(
        pan_ease_type, mPanEaseType, mPanEaseType = (EaseType)_val.Int();
        mPanEaseFunc = GetEaseFunction(mPanEaseType)
    )
    SYNC_PROP_SET(pan_ease_param, mPanEaseParam, mPanEaseParam = _val.Float())
    SYNC_PROP_SET(pan_target, mTargetPan, mTargetPan = _val.Float())
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
    SYNC_PROP_SET(transpose_target, mTargetTranspose, mTargetTranspose = _val.Float())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Fader)
    MILO_ASSERT_FMT(mLocalName.Null(), "can't save local fader");
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object);
    bs << mVolumeDb;
    bs << mVolumeEaseType;
    bs << mVolumeEaseParam;
    bs << mTargetVolumeDb;
    bs << mPan;
    bs << mPanEaseType;
    bs << mPanEaseParam;
    bs << mTargetPan;
    bs << mTranspose;
    bs << mTransposeEaseType;
    bs << mTransposeEaseParam;
    bs << mTargetTranspose;
END_SAVES

BEGIN_COPYS(Fader)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(Fader)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            SetVolume(c->mVolumeDb);
            COPY_MEMBER(mVolumeEaseType)
            mVolumeEaseFunc = GetEaseFunction(mVolumeEaseType);
            COPY_MEMBER(mVolumeEaseParam)
            COPY_MEMBER(mTargetVolumeDb)
            SetPan(c->mPan);
            COPY_MEMBER(mPanEaseType)
            mPanEaseFunc = GetEaseFunction(mPanEaseType);
            COPY_MEMBER(mPanEaseParam)
            COPY_MEMBER(mTargetPan)
            SetTranspose(c->mTranspose);
            COPY_MEMBER(mTransposeEaseType)
            mTransposeEaseFunc = GetEaseFunction(mTransposeEaseType);
            COPY_MEMBER(mTransposeEaseParam)
            COPY_MEMBER(mTargetTranspose)
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(Fader)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    float volume;
    d >> volume;
    SetVolume(volume);
    if (d.rev >= 1) {
        d >> (int &)mVolumeEaseType;
        mVolumeEaseFunc = GetEaseFunction(mVolumeEaseType);
        d >> mVolumeEaseParam;
        d >> mTargetVolumeDb;
        d >> mPan;
        d >> (int &)mPanEaseType;
        mPanEaseFunc = GetEaseFunction(mPanEaseType);
        d >> mPanEaseParam;
        d >> mTargetPan;
        d >> mTranspose;
        d >> (int &)mTransposeEaseType;
        mTransposeEaseFunc = GetEaseFunction(mTransposeEaseType);
        d >> mTransposeEaseParam;
        d >> mTargetTranspose;
    }
END_LOADS

void Fader::SynthPoll() {
    float div = mTimer.SplitMs() / mDurationMs;
    if (div >= 1) {
        CancelFade();
        div = 1;
    }

    // all of these are actually Interps, but using Interp causes regswaps, so...lol
    float volume = (mTargetVolumeDb - mStartVolumeDb)
            * Clamp(0.0f, 1.0f, mVolumeEaseFunc(div, mVolumeEaseParam, 0))
        + mStartVolumeDb;
    float pan =
        (mTargetPan - mStartPan) * Clamp(0.0f, 1.0f, mPanEaseFunc(div, mPanEaseParam, 0))
        + mStartPan;
    float transpose = (mTargetTranspose - mStartTranspose)
            * Clamp(0.0f, 1.0f, mTransposeEaseFunc(div, mTransposeEaseParam, 0))
        + mStartTranspose;
    UpdateValue(volume, pan, transpose);
}

void Fader::DoFade(float durationMs) {
    MILO_ASSERT(durationMs >= 0.0f, 0x5B);
    mDurationMs = durationMs;
    mStartVolumeDb = mVolumeDb;
    mStartPan = mPan;
    mStartTranspose = mTranspose;
    CancelFade();
    StartPolling();
    mTimer.Restart();
    MILO_ASSERT(mTimer.Running(), 0x65);
}

float Fader::GetDuckedVolume() const {
    float vol = 0;
    FOREACH (it, mDuckedVolumes) {
        if (*it < vol) {
            vol = *it;
        }
    }
    return vol;
}

void Fader::DoFade(float targetVol, float durationMs) {
    mTargetVolumeDb = targetVol;
    DoFade(durationMs);
}

void Fader::UpdateValue(float vol, float pan, float transpose) {
    mVolumeDb = vol;
    mPan = pan;
    mTranspose = transpose;
    FOREACH (it, mClients) {
        (*it)->SetDirty();
    }
}

void Fader::RemoveDuckedVolume(float f1) {
    std::list<float>::iterator it = mDuckedVolumes.begin();
    for (; it != mDuckedVolumes.end(); ++it) {
        if (*it == f1) {
            mDuckedVolumes.erase(it);
            FOREACH (it, mClients) {
                (*it)->SetDirty();
            }
            return;
        }
    }
}

void Fader::SetVolume(float vol) {
    CancelFade();
    UpdateValue(vol, mPan, mTranspose);
}

void Fader::SetPan(float pan) {
    CancelFade();
    UpdateValue(mVolumeDb, pan, mTranspose);
}

void Fader::SetTranspose(float transpose) {
    CancelFade();
    UpdateValue(mVolumeDb, mPan, transpose);
}

void Fader::AddDuckedVolume(float vol) {
    mDuckedVolumes.push_back(vol);
    FOREACH (it, mClients) {
        (*it)->SetDirty();
    }
}

#pragma endregion
#pragma region FaderGroup

FaderGroup::FaderGroup(Hmx::Object *owner) : mFaders(owner), mDirty(true) {}

FaderGroup::~FaderGroup() {
    while (!mFaders.empty()) {
        Fader *frontObj = mFaders.front();
        mFaders.pop_front();
        frontObj->RemoveClient(this);
        if (!frontObj->GetLocalName().Null()) {
            delete frontObj;
        }
    }
}

void FaderGroup::SetDirty() { mDirty = true; }
void FaderGroup::ClearDirty() { mDirty = false; }
bool FaderGroup::Dirty() { return mDirty; }

float FaderGroup::GetPan() const {
    float pan = 0;
    FOREACH (it, mFaders) {
        pan += (*it)->GetPan();
    }
    return pan;
}

float FaderGroup::GetTranspose() const {
    float transpose = 0;
    FOREACH (it, mFaders) {
        transpose += (*it)->GetTranspose();
    }
    return transpose;
}

Fader *FaderGroup::FindLocal(Symbol name, bool fail) {
    if (!name.Null()) {
        FOREACH (it, mFaders) {
            if ((*it)->GetLocalName() == name) {
                return *it;
            }
        }
    }
    if (fail) {
        MILO_FAIL("bad local fader: %s", name);
    }
    return nullptr;
}

float FaderGroup::GetVolume() const {
    float vol = 0;
    FOREACH (it, mFaders) {
        Fader *fader = *it;
        float duckedVol = 0;
        FOREACH (cit, fader->mDuckedVolumes) {
            if (*cit < duckedVol) {
                duckedVol = *cit;
            }
        }
        vol += fader->mVolumeDb + duckedVol;
    }
    return vol;
}

void FaderGroup::GetVal(float &vol, float &pan, float &transpose) const {
    vol = 0;
    pan = 0;
    transpose = 0;
    FOREACH (it, mFaders) {
        Fader *fader = *it;
        float val = 0;
        FOREACH (cit, fader->mDuckedVolumes) {
            if (*cit < val) {
                val = *cit;
            }
        }
        vol += fader->mVolumeDb + val;
        pan += (*it)->mPan;
        transpose += (*it)->mTranspose;
    }
}

bool PropSync(FaderGroup &grp, DataNode &node, DataArray *prop, int i, PropOp op) {
    ObjPtrList<Fader> pList(grp.Faders());
    for (ObjPtrList<Fader>::iterator it = pList.begin(); it != pList.end(); it) {
        if ((*it)->Dir() && (*it)->GetLocalName().Null())
            ++it;
        else
            it = pList.erase(it);
    }
    bool sync = PropSync(pList, node, prop, i, op);
    for (ObjPtrList<Fader>::iterator it = grp.Faders().begin(); it != grp.Faders().end();
         it) {
        Fader *f = *it++;
        if (f->Dir() && f->GetLocalName().Null())
            grp.Remove(f);
    }
    for (ObjPtrList<Fader>::iterator it = pList.begin(); it != pList.end(); ++it) {
        grp.Add(*it);
    }
    return sync;
}

Fader *FaderGroup::AddLocal(Symbol name) {
    MILO_ASSERT(!name.Null(), 0x14F);
    FOREACH (it, mFaders) {
        if ((*it)->GetLocalName() == name) {
            return *it;
        }
    }
    Fader *f = Hmx::Object::New<Fader>();
    f->SetLocalName(name);
    f->AddClient(this);
    mFaders.push_back(f);
    mDirty = true;
    return f;
}

void FaderGroup::Add(Fader *f) {
    FOREACH (it, mFaders) {
        if (*it == f)
            return;
    }
    if (f) {
        f->AddClient(this);
        mFaders.push_back(f);
    }
    mDirty = true;
}

void FaderGroup::Remove(Fader *f) {
    FOREACH (it, mFaders) {
        if (*it == f) {
            f->RemoveClient(this);
            mFaders.erase(it);
            mDirty = true;
            return;
        }
    }
}

void FaderGroup::Save(BinStream &bs) {
    bs << 0;
    ObjPtrList<Fader> faders(mFaders);
    for (auto it = faders.begin(); it != faders.end(); it) {
        if ((*it)->Dir() && (*it)->GetLocalName().Null()) {
            ++it;
        } else {
            it = faders.erase(it);
        }
    }
    bs << faders;
}

#define kGroupRev 0

void FaderGroup::Load(BinStream &bs) {
    int rev;
    bs >> rev;
    MILO_ASSERT(rev <= kGroupRev, 0x213);
    ObjPtrList<Fader> pList(mFaders.Owner());
    bs >> pList;
    for (ObjPtrList<Fader>::iterator it = mFaders.begin(); it != mFaders.end(); it) {
        Fader *f = *it++;
        if (f->Dir() && f->GetLocalName().Null())
            Remove(f);
    }
    for (ObjPtrList<Fader>::iterator it = pList.begin(); it != pList.end(); ++it) {
        Add(*it);
    }
}
