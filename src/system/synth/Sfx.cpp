#include "synth/Sfx.h"
#include "SampleInst.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "synth/MoggClip.h"
#include "synth/Sequence.h"
#include "synth/Synth.h"
#include "synth/SynthSample.h"
#include "synth/Utl.h"

#pragma region SfxInst

SfxInst::SfxInst(Sfx *sfx) : SeqInst(sfx), mSfx(sfx), mStartProgress(0) {
    FOREACH (it, sfx->SfxMaps()) {
        SampleInst *inst = nullptr;
        if (it->Sample()) {
            inst = it->Sample()->NewInst(false, 0, -1);
        }
        if (inst) {
            inst->SetBankVolume(it->Volume() + mRandVol);
            inst->SetBankPan(it->Pan() + mRandPan);
            inst->SetBankSpeed(CalcSpeedFromTranspose(it->Transpose() + mRandTp));
            inst->SetFXCore(it->GetFXCore());
            inst->SetADSR(it->ADSR());
            inst->SetSend(sfx->GetSend());
            inst->SetReverbMixDb(sfx->GetReverbMixDb());
            inst->SetReverbEnable(sfx->GetReverbEnable());
            mSamples.push_back(inst);
        }
    }
    FOREACH (it, mSfx->MoggClipMaps()) {
        if (it->GetMoggClip()) {
            it->GetMoggClip()->SetSend(sfx->GetSend());
        }
    }
}

SfxInst::~SfxInst() { DeleteAll(mSamples); }

void SfxInst::Stop() {
    FOREACH (it, mSamples) {
        (*it)->Stop(false);
    }
    FOREACH (it, mSfx->MoggClipMaps()) {
        if (it->GetMoggClip()) {
            it->GetMoggClip()->Stop(false);
        }
    }
}

bool SfxInst::IsRunning() {
    FOREACH (it, mSamples) {
        if ((*it)->IsPlaying())
            return true;
    }
    FOREACH (it, mSfx->MoggClipMaps()) {
        if (it->GetMoggClip() && it->GetMoggClip()->GetStream()) {
            return true;
        }
    }
    return false;
}

#pragma endregion
#pragma region Sfx

Sfx::Sfx()
    : mMaps(this), mMoggClipMaps(this), mSend(this), mReverbMixDb(kDbSilence),
      mReverbEnable(false), mSfxInsts(this) {
    mFaders.Add(TheSynth->MasterFader());
    mFaders.Add(TheSynth->SfxFader());
}

BEGIN_HANDLERS(Sfx)
    HANDLE_ACTION(add_map, mMaps.push_back())
    HANDLE_SUPERCLASS(Sequence)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(ADSRImpl)
    SYNC_PROP(attack_mode, (int &)o.mAttackMode)
    SYNC_PROP(attack_rate, o.mAttackRate)
    SYNC_PROP(decay_rate, o.mDecayRate)
    SYNC_PROP(sustain_mode, (int &)o.mSustainMode)
    SYNC_PROP(sustain_rate, o.mSustainRate)
    SYNC_PROP(sustain_level, o.mSustainLevel)
    SYNC_PROP(release_mode, (int &)o.mReleaseMode)
    SYNC_PROP(release_rate, o.mReleaseRate)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(SfxMap)
    SYNC_PROP(sample, o.mSample)
    SYNC_PROP(volume, o.mVolume)
    SYNC_PROP(pan, o.mPan)
    SYNC_PROP(transpose, o.mTranspose)
    SYNC_PROP(fx_core, (int &)o.mFXCore)
    SYNC_PROP(adsr, o.mADSR)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(MoggClipMap)
    SYNC_PROP(moggclip, o.mMoggClip)
    SYNC_PROP(volume, o.mVolume)
    SYNC_PROP(pan, o.mPan)
    SYNC_PROP(pan_width, o.mPanWidth)
    SYNC_PROP(is_stereo, o.mIsStereo)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(Sfx)
    SYNC_PROP(sfxmaps, mMaps)
    SYNC_PROP(moggclip_maps, mMoggClipMaps)
    SYNC_PROP_SET(send, GetSend(), SetSend(_val.Obj<FxSend>()))
    SYNC_PROP_SET(reverb_mix_db, GetReverbMixDb(), SetReverbMixDb(_val.Float()))
    SYNC_PROP_SET(reverb_enable, GetReverbEnable(), SetReverbEnable(_val.Int()))
    SYNC_PROP(faders, mFaders)
    SYNC_SUPERCLASS(Sequence)
END_PROPSYNCS

BEGIN_SAVES(Sfx)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(Sequence)
    bs << mMaps;
    bs << mMoggClipMaps;
    bs << mSend;
    mFaders.Save(bs);
    bs << mReverbMixDb;
    bs << mReverbEnable;
END_SAVES

BEGIN_COPYS(Sfx)
    COPY_SUPERCLASS(Sequence)
    CREATE_COPY(Sfx)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mMaps)
            COPY_MEMBER(mMoggClipMaps)
        }
        COPY_MEMBER(mSend)
        COPY_MEMBER(mReverbMixDb)
        COPY_MEMBER(mReverbEnable)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0xD, 0)

BEGIN_LOADS(Sfx)
    LOAD_REVS(bs)
    ASSERT_REVS(0xD, 0)
    if (d.rev >= 6) {
        LOAD_SUPERCLASS(Sequence)
    } else if (d.rev >= 2) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    d >> mMaps;
    if (d.rev >= 10) {
        d >> mMoggClipMaps;
    }
    if (d.rev > 4) {
        d >> mSend;
        if (d.rev <= 7) {
            int x;
            d >> x;
        }
    }
    if (d.rev >= 9) {
        mFaders.Load(d.stream);
    }
    if (d.rev >= 0xC) {
        d >> mReverbMixDb >> mReverbEnable;
    }
END_LOADS

SeqInst *Sfx::MakeInstImpl() {
    SfxInst *inst = new SfxInst(this);
    mSfxInsts.push_back(inst);
    return inst;
}

void Sfx::Pause(bool b1) {
    FOREACH (it, mSfxInsts) {
        (*it)->Pause(b1);
    }
}

void Sfx::SetSend(FxSend *send) {
    mSend = send;
    FOREACH (it, mSfxInsts) {
        (*it)->SetSend(mSend);
    }
}

void Sfx::SetReverbMixDb(float f) {
    mReverbMixDb = f;
    FOREACH (it, mSfxInsts) {
        (*it)->SetReverbMixDb(mReverbMixDb);
    }
}

void Sfx::SetReverbEnable(bool b) {
    mReverbEnable = b;
    FOREACH (it, mSfxInsts) {
        (*it)->SetReverbEnable(mReverbEnable);
    }
}
