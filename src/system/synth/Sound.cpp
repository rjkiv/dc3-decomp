#include "synth/Sound.h"
#include "SynthSample.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "synth/FxSend.h"
#include "synth/MoggClip.h"
#include "synth/PlayableSample.h"
#include "synth/SampleInst.h"
#include "synth/Synth.h"
#include "synth/SynthSample.h"
#include "synth/Utl.h"
#include "utl/Std.h"

const float sSpeedCaps[2] = { 0.00390625f, 4.0f };

Sound::Sound()
    : mVol(0), mSpeed(1), mPan(0), mSend(this), mReverbMixDb(kDbSilence),
      mReverbEnable(0), mCanStop(1), mSynthSample(this), mMoggClip(this), mADSR(this),
      mFaders(this), mDuckerGrp(this), mLoop(0), mLoopStartSamp(0), mLoopEndSamp(-1),
      mMaxPolyphony(0), mAddZombiesOnDelete(0), mEventReceiver(this) {
    mFaders.Add(TheSynth->MasterFader());
}

Sound::~Sound() {
    if (mAddZombiesOnDelete) {
        FOREACH (it, mInsts) {
            (*it)->Stop(true);
            TheSynth->AddZombie(static_cast<SampleInst *>(*it));
        }
    }
}

BEGIN_HANDLERS(Sound)
    HANDLE(play, OnPlay)
    HANDLE(disable_pan, DisablePan)
    HANDLE_ACTION(stop, Stop(nullptr, _msg->Size() == 4 ? _msg->Int(3) : false))
    HANDLE_ACTION(add_fader, mFaders.Add(_msg->Obj<Fader>(2)))
    HANDLE_ACTION(
        end_loop, EndLoop(_msg->Size() == 3 ? _msg->Obj<Hmx::Object>(2) : nullptr)
    )
    HANDLE_ACTION(add_ducker, mDuckerGrp.Add(_msg->Obj<Fader>(2), _msg->Float(3)))
    HANDLE_ACTION(remove_ducker, mDuckerGrp.Remove(_msg->Obj<Fader>(2)))
    HANDLE_EXPR(is_mogg_ready, IsMoggReady())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Sound)
    SYNC_SUPERCLASS(Hmx::Object)
    SYNC_PROP(faders, mFaders)
    SYNC_PROP(duckers, mDuckerGrp)
    SYNC_PROP(envelope, mADSR)
    SYNC_PROP(loop, mLoop)
    SYNC_PROP(loop_start, mLoopStartSamp)
    SYNC_PROP(loop_end, mLoopEndSamp)
    SYNC_PROP(max_polyphony, mMaxPolyphony)
    SYNC_PROP_SET(volume, mVol, SetVolume(_val.Float(), nullptr))
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
    bs << mVol;
    bs << CalcTransposeFromSpeed(mSpeed);
    bs << mPan;
    bs << mCanStop;
    bs << mSynthSample;
    bs << mReverbMixDb;
    bs << mReverbEnable;
    bs << mADSR;
    bs << mSend;
    bs << mMoggClip;
    bs << mLoop;
    bs << mLoopStartSamp;
    bs << mLoopEndSamp;
    bs << mMaxPolyphony;
    mFaders.Save(bs);
    mDuckerGrp.Save(bs);
END_SAVES

BEGIN_COPYS(Sound)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(Sound)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mVol)
            COPY_MEMBER(mSpeed)
            COPY_MEMBER(mPan)
            COPY_MEMBER(mCanStop)
            COPY_MEMBER(mSynthSample)
            mAddZombiesOnDelete = mSynthSample;
            COPY_MEMBER(mMoggClip)
            COPY_MEMBER(mReverbMixDb)
            COPY_MEMBER(mReverbEnable)
            COPY_MEMBER(mADSR)
            COPY_MEMBER(mSend)
            COPY_MEMBER(mLoop)
            COPY_MEMBER(mLoopStartSamp)
            COPY_MEMBER(mLoopEndSamp)
            COPY_MEMBER(mMaxPolyphony)
            FOREACH (it, c->mFaders.Faders()) {
                mFaders.Add(*it);
            }
            COPY_MEMBER(mDuckerGrp)
            MILO_ASSERT(! (mSynthSample && mMoggClip), 0xB9);
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(9, 0)

BEGIN_LOADS(Sound)
    LOAD_REVS(bs)
    ASSERT_REVS(9, 0)
    Hmx::Object::Load(bs);
    bs >> mVol;
    float transpose;
    bs >> transpose;
    mSpeed = Clamp(sSpeedCaps[0], sSpeedCaps[1], CalcSpeedFromTranspose(transpose));
    bs >> mPan;
    d >> mCanStop;
    bs >> mSynthSample;
    mAddZombiesOnDelete = mSynthSample;
    if (d.rev >= 2) {
        bs >> mReverbMixDb;
        d >> mReverbEnable;
    }
    mADSR = nullptr;
    if (d.rev >= 3) {
        bs >> mADSR;
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
        bs >> mLoopStartSamp;
        bs >> mLoopEndSamp;
    }
    if (d.rev >= 7) {
        bs >> mMaxPolyphony;
    }
    if (d.rev >= 8) {
        mFaders.Load(bs);
    }
    if (d.rev >= 9) {
        mDuckerGrp.Load(bs);
    }
    MILO_ASSERT(! (mSynthSample && mMoggClip), 0x95);
END_LOADS

const char *Sound::GetSoundDisplayName() { return MakeString("Sequence: %s", Name()); }

void Sound::SynthPoll() {
    float deltaMs = TheTaskMgr.DeltaSeconds() * 1000.0f;
    for (auto it = mDelayedSounds.begin(); it != mDelayedSounds.end();) {
        (*it)->timeLeftMs -= deltaMs;
        if ((*it)->timeLeftMs <= 0) {
            Play((*it)->volume, (*it)->pan, (*it)->transpose, this, 0);
            delete *it;
            it = mDelayedSounds.erase(it);
        } else {
            it++;
        }
    }
    for (auto it = mInsts.begin(); it != mInsts.end();) {
        PlayableSample *cur = *it;
        it++;
        if (mAddZombiesOnDelete || mMoggClip) {
            if (cur->DonePlaying()) {
                mInsts.erase(it);
            }
        } else {
            mDuckerGrp.Unduck();
            CancelPolling();
        }
    }
    if (mFaders.Dirty()) {
        FOREACH (it, mInsts) {
            float faderVol, faderPan, faderTranspose;
            mFaders.GetVal(faderVol, faderPan, faderTranspose);
            (*it)->SetVolume(mVol + faderVol);
            (*it)->SetPan(Clamp(-4.0f, sSpeedCaps[1], mPan + faderPan));
            (*it)->SetSpeed(Clamp(
                sSpeedCaps[0],
                sSpeedCaps[1],
                CalcSpeedFromTranspose(faderTranspose) * mSpeed
            ));
        }
        mFaders.ClearDirty();
    }
    if (mInsts.empty() && mDelayedSounds.empty()) {
        mDuckerGrp.Unduck();
        CancelPolling();
    }
}

void Sound::Play(
    float volume, float pan, float transpose, Hmx::Object *obj, float delayMs
) {
    if (Name() && strstr(Name(), "camp_gameplay_failure")) {
        MILO_LOG(
            "[EH] BZ-64344 Playing sound with camp_gameplay_failure in it: '%s'\n", Name()
        );
    }
    MILO_ASSERT(delayMs >= 0.f, 0x1B7);

    if (delayMs > 0.0f) {
        mDelayedSounds.push_back(new DelayArgs(volume, pan, transpose, obj, delayMs));
        StartPolling();
    } else {
        PlayableSample *sample = nullptr;
        if (mSynthSample) {
            sample = mSynthSample->NewInst(mLoop, mLoopStartSamp, mLoopEndSamp);
            if (sample) {
                sample->SetSend(mSend);
            }
        } else if (mMoggClip) {
            sample = mMoggClip;
            mMoggClip->SetSend(mSend);
            mMoggClip->SetLoop(mLoop, mLoopStartSamp, mLoopEndSamp);
            mInsts.clear();
        } else {
            return;
        }
        if (sample) {
            mDuckerGrp.Duck();
            mInsts.push_back(sample);
            StartPolling();
            float faderVol, faderPan, faderTranspose;
            mFaders.GetVal(faderVol, faderPan, faderTranspose);
            sample->Play(mVol + faderVol + volume);
            sample->SetPan(Clamp(-4.0f, sSpeedCaps[1], mPan + faderPan + pan));
            sample->SetSpeed(Clamp(
                sSpeedCaps[0],
                sSpeedCaps[1],
                CalcSpeedFromTranspose(faderTranspose + transpose) * mSpeed
            ));
            sample->SetEventReceiver(obj ? obj : mEventReceiver);
            if (mADSR) {
                sample->SetADSR(mADSR->Impl());
            } else {
                sample->SetADSR(*TheSynth->DefaultADSR());
            }
            sample->SetReverbMixDb(mReverbMixDb);
            sample->SetReverbEnable(mReverbEnable);
            if (mMaxPolyphony != 0) {
                auto it = mInsts.begin();
                for (int i = 0; i < (int)mInsts.size() - mMaxPolyphony; i++) {
                    (*it)->Stop(false);
                    ++it;
                }
            }
        } else {
            MILO_LOG("Sound::Play : '%s' **** NOT FOUND\n", PathName(this));
        }
        TheSynth->SendToPlayHandlers(this);
    }
}

void Sound::Stop(Hmx::Object *obj, bool b2) {
    for (auto it = mDelayedSounds.begin(); it != mDelayedSounds.end();) {
        delete *it;
        mDelayedSounds.erase(it++);
    }
    if ((mAddZombiesOnDelete || mMoggClip) && (mCanStop || b2)) {
        if (!obj) {
            for (auto it = mInsts.begin(); it != mInsts.end(); it) {
                PlayableSample *cur = *it;
                it++;
                cur->Stop(b2);
                Hmx::Object *eventReceiver = cur->GetEventReceiver();
                if (eventReceiver) {
                    static Message msg("on_marker_event", Symbol("interrupted"));
                    eventReceiver->Handle(msg, false);
                }
            }
        } else {
            for (auto it = mInsts.begin(); it != mInsts.end(); it) {
                if ((*it)->GetEventReceiver() == obj) {
                    PlayableSample *cur = *it;
                    it++;
                    cur->Stop(b2);
                    static Message msg("on_marker_event", Symbol("interrupted"));
                    obj->Handle(msg, false);
                } else {
                    it++;
                }
            }
        }
    }
}

void Sound::Pause(bool b1) {
    FOREACH (it, mInsts) {
        (*it)->Pause(b1);
    }
}

bool Sound::IsPlaying() const { return !mInsts.empty() || !mDelayedSounds.empty(); }

void Sound::SetVolume(float vol, Hmx::Object *obj) {
    float faderVol = mFaders.GetVolume();
    if (obj) {
        FOREACH (it, mInsts) {
            if ((*it)->GetEventReceiver() == obj) {
                (*it)->SetVolume(faderVol + vol);
                return;
            }
        }
    } else {
        mVol = vol;
        FOREACH (it, mInsts) {
            (*it)->SetVolume(faderVol + vol);
        }
    }
}

void Sound::SetReverbMixDb(float db) {
    mReverbMixDb = db;
    FOREACH (it, mInsts) {
        (*it)->SetReverbMixDb(mReverbMixDb);
    }
}

void Sound::SetReverbEnable(bool enable) {
    mReverbEnable = enable;
    FOREACH (it, mInsts) {
        (*it)->SetReverbEnable(mReverbEnable);
    }
}

void Sound::EndLoop(Hmx::Object *obj) {
    if (obj) {
        FOREACH (it, mInsts) {
            if ((*it)->GetEventReceiver() == obj) {
                (*it)->EndLoop();
            }
        }
    } else {
        FOREACH (it, mInsts) {
            (*it)->EndLoop();
        }
    }
}

float Sound::ElapsedTime() {
    if (mInsts.empty()) {
        return 0;
    } else {
        return mInsts.front()->ElapsedTime();
    }
}

void Sound::SetSend(FxSend *send) {
    mSend = send;
    if (send) {
        send->RebuildChain();
    }
    FOREACH (it, mInsts) {
        (*it)->SetSend(mSend);
    }
}

void Sound::SetSynthSample(SynthSample *sample) {
    Stop(nullptr, true);
    mMoggClip = nullptr;
    mSynthSample = sample;
    mAddZombiesOnDelete = true;
}

void Sound::SetMoggClip(MoggClip *clip) {
    Stop(nullptr, true);
    mSynthSample = nullptr;
    mMoggClip = clip;
    mAddZombiesOnDelete = false;
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
        if (mInsts.empty()) {
            Play(0, 0, 0, nullptr, 0);
        }
        break;
    default:
        MILO_FAIL("Invalid argument to OnTriggerSound: %d\n", arg);
        break;
    }
}

bool Sound::DisablePan(DataArray *a) {
    if (mMoggClip && mMoggClip->GetNumChannels() > 1) {
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

void Sound::SetSpeed(float f1, Hmx::Object *eventReceiver) {
    float speedTranspose = CalcSpeedFromTranspose(mFaders.GetTranspose());
    float clamped = Clamp(sSpeedCaps[0], sSpeedCaps[1], f1);
    if (eventReceiver) {
        FOREACH (it, mInsts) {
            if ((*it)->GetEventReceiver() == eventReceiver) {
                (*it)->SetSpeed(
                    Clamp(sSpeedCaps[0], sSpeedCaps[1], speedTranspose * clamped)
                );
                return;
            }
        }
    } else {
        mSpeed = clamped;
        FOREACH (it, mInsts) {
            (*it)->SetSpeed(Clamp(sSpeedCaps[0], sSpeedCaps[1], speedTranspose * mSpeed));
        }
    }
}

void Sound::SetPan(float pan, Hmx::Object *eventReceiver) {
    bool clipOk;
    if (mMoggClip && mMoggClip->GetNumChannels() > 1) {
        clipOk = true;
        mPan = 0;
    } else {
        clipOk = false;
    }

    if (!clipOk) {
        float faderPan = mFaders.GetPan();
        if (eventReceiver) {
            FOREACH (it, mInsts) {
                if ((*it)->GetEventReceiver() == eventReceiver) {
                    (*it)->SetPan(Clamp(-4.0f, 4.0f, faderPan + pan));
                    return;
                }
            }
        } else {
            mPan = pan;
            FOREACH (it, mInsts) {
                (*it)->SetPan(Clamp(-4.0f, 4.0f, faderPan + pan));
            }
        }
    }
}
