#include "synth/Sound.h"
#include "SynthSample.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "synth/FxSend.h"
#include "synth/MoggClip.h"
#include "synth/PlayableSample.h"
#include "synth/SampleInst.h"
#include "synth/Synth.h"
#include "synth/SynthSample.h"
#include "synth/Utl.h"
#include "utl/Std.h"

Sound::Sound()
    : mVolume(0), mSpeed(1), mPan(0), mSend(this), mReverbMixDb(kDbSilence),
      mReverbEnable(0), unk3d(1), mSynthSample(this), mMoggClip(this), mEnvelope(this),
      mFaders(this), mDuckers(this), mLoop(0), mLoopStart(0), mLoopEnd(-1),
      mMaxPolyphony(0), unkb4(0), unkb8(this) {
    mFaders.Add(TheSynth->MasterFader());
}

Sound::~Sound() {
    if (unkb4) {
        FOREACH (it, mSamples) {
            (*it)->Stop(true);
            TheSynth->AddZombie(static_cast<SampleInst *>(*it));
        }
    }
}

BEGIN_HANDLERS(Sound)
    HANDLE(play, OnPlay)
    HANDLE_EXPR(disable_pan, DisablePan(nullptr))
    HANDLE_ACTION(stop, Stop(nullptr, _msg->Size() == 4 ? _msg->Int(3) : false))
    HANDLE_ACTION(add_fader, mFaders.Add(_msg->Obj<Fader>(2)))
    HANDLE_ACTION(
        end_loop, EndLoop(_msg->Size() == 3 ? _msg->Obj<Hmx::Object>(2) : nullptr)
    )
    HANDLE_ACTION(add_ducker, mDuckers.Add(_msg->Obj<Fader>(2), _msg->Float(3)))
    HANDLE_ACTION(remove_ducker, mDuckers.Remove(_msg->Obj<Fader>(2)))
    HANDLE_EXPR(is_mogg_ready, IsMoggReady())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Sound)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_PROP(faders, mFaders)
    SYNC_PROP(duckers, mDuckers)
    SYNC_PROP(envelope, mEnvelope)
    SYNC_PROP(loop, mLoop)
    SYNC_PROP(loop_start, mLoopStart)
    SYNC_PROP(loop_end, mLoopEnd)
    SYNC_PROP(max_polyphony, mMaxPolyphony)
    SYNC_PROP_SET(volume, mVolume, SetVolume(_val.Float(), nullptr))
    SYNC_PROP_SET(
        transpose,
        CalcTransposeFromSpeed(mSpeed),
        SetSpeed(CalcSpeedFromTranspose(_val.Float()), nullptr)
    )
    SYNC_PROP_SET(pan, mPan, SetPan(_val.Float(), nullptr))
    SYNC_PROP_SET(send, mSend.Ptr(), SetSend(_val.Obj<FxSend>()))
    SYNC_PROP_SET(reverb_mix_db, mReverbMixDb, SetReverbMixDb(_val.Float()))
    SYNC_PROP_SET(reverb_enable, mReverbEnable, SetReverbEnable(_val.Int()))
    SYNC_PROP_SET(sample, mSynthSample.Ptr(), SetSynthSample(_val.Obj<SynthSample>()))
    SYNC_PROP_SET(mogg, mMoggClip.Ptr(), SetMoggClip(_val.Obj<MoggClip>()))
    SYNC_PROP_SET(trigger_sound, 0, OnTriggerSound(_val.Int()))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Sound)
    SAVE_REVS(9, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mVolume;
    bs << CalcTransposeFromSpeed(mSpeed);
    bs << mPan;
    bs << unk3d;
    bs << mSynthSample;
    bs << mReverbMixDb;
    bs << mReverbEnable;
    bs << mEnvelope;
    bs << mSend;
    bs << mMoggClip;
    bs << mLoop;
    bs << mLoopStart;
    bs << mLoopEnd;
    bs << mMaxPolyphony;
    mFaders.Save(bs);
    mDuckers.Save(bs);
END_SAVES

BEGIN_COPYS(Sound)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(Sound)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mVolume)
            COPY_MEMBER(mSpeed)
            COPY_MEMBER(mPan)
            COPY_MEMBER(unk3d)
            COPY_MEMBER(mSynthSample)
            unkb4 = mSynthSample;
            COPY_MEMBER(mMoggClip)
            COPY_MEMBER(mReverbMixDb)
            COPY_MEMBER(mReverbEnable)
            COPY_MEMBER(mEnvelope)
            COPY_MEMBER(mSend)
            COPY_MEMBER(mLoop)
            COPY_MEMBER(mLoopStart)
            COPY_MEMBER(mLoopEnd)
            COPY_MEMBER(mMaxPolyphony)
            FOREACH (it, c->mFaders.Faders()) {
                mFaders.Add(*it);
            }
            COPY_MEMBER(mDuckers)
            MILO_ASSERT(! (mSynthSample && mMoggClip), 0xB9);
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(Sound)
    LOAD_REVS(bs)
    ASSERT_REVS(9, 0)
    Hmx::Object::Load(bs);
    bs >> mVolume;
    float transpose;
    bs >> transpose;
    mSpeed = Clamp(0.00390625f, CalcSpeedFromTranspose(transpose), 4.0f);
    bs >> mPan;
    d >> unk3d;
    bs >> mSynthSample;
    unkb4 = mSynthSample;
    if (d.rev >= 2) {
        bs >> mReverbMixDb;
        d >> mReverbEnable;
    }
    mEnvelope = nullptr;
    if (d.rev >= 3) {
        bs >> mEnvelope;
    }
    mSend = nullptr;
    if (d.rev >= 4) {
        bs >> mSend;
    }
    if (d.rev >= 5) {
        bs >> mMoggClip;
    }
    if (d.rev >= 6) {
        d >> mLoop;
        bs >> mLoopStart;
        bs >> mLoopEnd;
    }
    if (d.rev >= 7) {
        bs >> mMaxPolyphony;
    }
    if (d.rev >= 8) {
        mFaders.Load(bs);
    }
    if (d.rev >= 9) {
        mDuckers.Load(bs);
    }
    MILO_ASSERT(! (mSynthSample && mMoggClip), 0x95);
END_LOADS

const char *Sound::GetSoundDisplayName() { return MakeString("Sequence: %s", Name()); }

void Sound::Play(float volume, float pan, float transpose, Hmx::Object *obj, float delayMs) {
    if (Name() && strstr(Name(), "camp_gameplay_failure")) {
        MILO_LOG(
            "[EH] BZ-64344 Playing sound with camp_gameplay_failure in it: '%s'\n", Name()
        );
    }
    MILO_ASSERT(delayMs >= 0.f, 0x1B7);

    if (delayMs > 0.0f) {
        DelayArgs *args = new DelayArgs;
        if (args) {
            args->unk0 = volume;
            args->unk4 = pan;
            args->unk8 = transpose;
            args->unkc = obj;
            args->unk10 = delayMs;
        }
        mDelayArgs.push_back(args);
        StartPolling();
    } else {
        PlayableSample *sample = nullptr;
        if (mSynthSample) {
            SampleInst *inst = mSynthSample->NewInst(mLoop, mLoopStart, mLoopEnd);
            sample = inst;
            if (sample) {
                sample->SetEventReceiver(obj);
            }
        } else if (mMoggClip) {
            sample = mMoggClip;
            sample->SetEventReceiver(obj);
            mMoggClip->SetLoop(mLoop, mLoopStart, mLoopEnd);
            mSamples.clear();
        }
        if (sample) {
            mDuckers.Duck();
            mSamples.push_back(sample);
            StartPolling();
            float faderVol, faderPan, faderTranspose;
            mFaders.GetVal(faderVol, faderPan, faderTranspose);
            sample->Play(mVolume + faderVol + volume);
            sample->SetPan(Clamp(-4.0f, 4.0f, mPan + faderPan + pan));
            sample->SetSpeed(Clamp(0.00390625f, 4.0f, CalcSpeedFromTranspose(faderTranspose + transpose) * mSpeed));
            sample->SetEventReceiver(obj ? obj : unkb8);
            if (mEnvelope) {
                sample->SetADSR(mEnvelope->Impl());
            } else {
                sample->SetADSR(*TheSynth->DefaultADSR());
            }
            sample->SetSend(mSend);
            sample->SetReverbMixDb(mReverbMixDb);
            sample->SetReverbEnable(mReverbEnable);
            if (mMaxPolyphony != 0) {
                auto it = mSamples.begin();
                for (int i = 0; i < (int)mSamples.size() - mMaxPolyphony; i++) {
                    (*it)->Pause(false);
                    ++it;
                }
            }
        } else {
            MILO_LOG("Sound::Play : '%s' **** NOT FOUND\n", PathName(this));
        }
    }
    TheSynth->SendToPlayHandlers(this);
}

void Sound::Stop(Hmx::Object *obj, bool b2) {
    for (auto it = mDelayArgs.begin(); it != mDelayArgs.end();) {
        delete *it;
        mDelayArgs.erase(it++);
    }
    if ((unkb4 || mMoggClip) && (unk3d || b2)) {
        if (!obj) {
            for (auto it = mSamples.begin(); it != mSamples.end(); it) {
                PlayableSample *cur = *it++;
                cur->Stop(b2);
                Hmx::Object *eventReceiver = cur->GetEventReceiver();
                if (eventReceiver) {
                    static Message msg("on_marker_event", Symbol("interrupted"));
                    eventReceiver->Handle(msg, false);
                }
            }
        } else {
            for (auto it = mSamples.begin(); it != mSamples.end(); ++it) {
                if ((*it)->GetEventReceiver() == obj) {
                    (*it)->Stop(b2);
                    static Message msg("on_marker_event", Symbol("interrupted"));
                    obj->Handle(msg, false);
                }
            }
        }
    }
}

void Sound::Pause(bool b1) {
    FOREACH (it, mSamples) {
        (*it)->Pause(b1);
    }
}

bool Sound::IsPlaying() const { return !mSamples.empty() || !mDelayArgs.empty(); }

void Sound::SetVolume(float vol, Hmx::Object *obj) {
    float faderVol = mFaders.GetVolume();
    if (obj) {
        FOREACH (it, mSamples) {
            if ((*it)->GetEventReceiver() == obj) {
                (*it)->SetVolume(faderVol + vol);
                return;
            }
        }
    } else {
        mVolume = vol;
        FOREACH (it, mSamples) {
            (*it)->SetVolume(faderVol + vol);
        }
    }
}

void Sound::SetReverbMixDb(float db) {
    mReverbMixDb = db;
    FOREACH (it, mSamples) {
        (*it)->SetReverbMixDb(mReverbMixDb);
    }
}

void Sound::SetReverbEnable(bool enable) {
    mReverbEnable = enable;
    FOREACH (it, mSamples) {
        (*it)->SetReverbEnable(mReverbEnable);
    }
}

void Sound::EndLoop(Hmx::Object *obj) {
    if (obj) {
        FOREACH (it, mSamples) {
            if ((*it)->GetEventReceiver() == obj) {
                (*it)->EndLoop();
            }
        }
    } else {
        FOREACH (it, mSamples) {
            (*it)->EndLoop();
        }
    }
}

float Sound::ElapsedTime() {
    if (mSamples.empty()) {
        return 0;
    } else {
        return mSamples.front()->ElapsedTime();
    }
}

void Sound::SetSend(FxSend *send) {
    mSend = send;
    if (send) {
        send->RebuildChain();
    }
    FOREACH (it, mSamples) {
        (*it)->SetSend(mSend);
    }
}

void Sound::SetSynthSample(SynthSample *sample) {
    Stop(nullptr, true);
    mMoggClip = nullptr;
    mSynthSample = sample;
    unkb4 = true;
}

void Sound::SetMoggClip(MoggClip *clip) {
    Stop(nullptr, true);
    mSynthSample = nullptr;
    mMoggClip = clip;
    unkb4 = false;
}

int Sound::NumMarkers() const {
    if (mSynthSample) {
        return mSynthSample->NumMarkers();
    } else
        return 0;
}

void Sound::OnTriggerSound(int arg) {
    switch (arg) {
    case 0:
        Stop(nullptr, false);
        break;
    case 1:
        Play(0, 0, 0, nullptr, 0);
        break;
    case 2:
        if (mSamples.empty()) {
            Play(0, 0, 0, nullptr, 0);
        }
        break;
    default:
        MILO_FAIL("Invalid argument to OnTriggerSound: %d\n", arg);
        break;
    }
}

bool Sound::DisablePan(DataArray *a) {
    if (mMoggClip && mMoggClip->NumChannels() > 1) {
        mPan = 0;
        return true;
    } else
        return false;
}

bool Sound::IsMoggReady() const { return mMoggClip && mMoggClip->IsReadyToPlay(); }

DataNode Sound::OnPlay(DataArray *a) {
    static Symbol volume("volume");
    static Symbol pan("pan");
    static Symbol transpose("transpose");
    float volumeArg = 0;
    float panArg = 0;
    float transposeArg = 0;
    a->FindData(volume, volumeArg, false);
    a->FindData(pan, panArg, false);
    a->FindData(transpose, transposeArg, false);
    Play(volumeArg, panArg, transposeArg, nullptr, 0);
    return 0;
}

SynthSample *Sound::Sample() { return mSynthSample; }
