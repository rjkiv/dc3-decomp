#include "synth/MidiInstrument.h"
#include "SampleZone.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"
#include "synth/Utl.h"

#pragma region NoteVoiceInst

NoteVoiceInst::NoteVoiceInst(
    MidiInstrument *owner,
    SampleZone *zone,
    unsigned char trigger,
    unsigned char ratio,
    int durFramesLeft,
    int glideID,
    float fineTune
)
    : mSample(nullptr), mVolume(0), mStartProgress(0), mTriggerNote(trigger),
      mCenterNote(zone->CenterNote()), mStarted(false), mStopped(false),
      mGlideID(glideID), mGlideFrames(0), mGlideToNote(0), mGlideFromNote(0),
      mGlideFramesLeft(-1), mFineTune(fineTune), mDurationFramesLeft(durFramesLeft),
      mOwner(owner) {
    if (zone->Sample()) {
        mSample = zone->Sample()->NewInst(false, 0, -1);
        float db = RatioToDb(ratio / 127.0f);
        mSample->SetBankVolume(zone->Volume() + db);
        mSample->SetBankPan(zone->Pan());
        mSample->SetBankSpeed(getCalculatedSpeed(mTriggerNote));
        mSample->SetFXCore(zone->GetFXCore());
        mSample->SetADSR(zone->ADSR());
        mSample->SetSend(owner->GetSend());
    }
}

NoteVoiceInst::~NoteVoiceInst() { RELEASE(mSample); }

void NoteVoiceInst::Start() {
    mStarted = true;
    mSample->SetStartProgress(mStartProgress);
    mSample->Play(mOwner->Faders().GetVolume() + mVolume);
}

void NoteVoiceInst::Stop() {
    mStopped = true;
    mSample->Stop(false);
}

bool NoteVoiceInst::IsRunning() { return mSample->IsPlaying(); }

void NoteVoiceInst::SetTranspose(float transpose) {
    float speed = CalcSpeedFromTranspose(transpose);
    mSample->SetSpeed(speed);
}

void NoteVoiceInst::UpdateVolume() {
    if (mSample && mOwner) {
        mSample->SetVolume(mOwner->Faders().GetVolume() + mVolume);
    }
}

void NoteVoiceInst::UpdatePan() {
    if (mSample && mOwner) {
        mSample->SetPan(mOwner->Faders().GetPan());
    }
}

void NoteVoiceInst::SetPan(float pan) { mSample->SetPan(pan); }

void NoteVoiceInst::SetVolume(float volume) {
    mVolume = volume;
    UpdateVolume();
}

float NoteVoiceInst::getCalculatedSpeed(float f1) {
    return CalcSpeedFromTranspose(mFineTune / 100.0f + (f1 - mCenterNote));
}

#pragma endregion
#pragma region MidiInstrument

MidiInstrument::MidiInstrument()
    : mMultiSampleMap(this), mPatchNumber(0), mSend(this), mReverbMixDb(kDbSilence),
      mReverbEnable(false), mActiveVoices(this), mFaders(this), mFineTuneCents(0) {
    mFaders.Add(TheSynth->MasterFader());
    mFaders.Add(TheSynth->InstFader());
}

MidiInstrument::~MidiInstrument() { mActiveVoices.DeleteAll(); }

BEGIN_HANDLERS(MidiInstrument)
    HANDLE_ACTION(add_map, mMultiSampleMap.push_back())
    HANDLE_ACTION(play_note, StartSample(_msg->Int(2), _msg->Int(3), _msg->Int(4), -1))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(SampleZone)
    SYNC_PROP(sample, o.mSample)
    SYNC_PROP(volume, o.mVolume)
    SYNC_PROP(pan, o.mPan)
    SYNC_PROP(centernote, o.mCenterNote)
    SYNC_PROP(minnote, o.mMinNote)
    SYNC_PROP(maxnote, o.mMaxNote)
    SYNC_PROP(minvelo, o.mMinVel)
    SYNC_PROP(maxvelo, o.mMaxVel)
    SYNC_PROP(fx_core, (int &)o.mFXCore)
    SYNC_PROP(adsr, o.mADSR)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(MidiInstrument)
    SYNC_PROP(multisamplemaps, mMultiSampleMap)
    SYNC_PROP_SET(send, GetSend(), SetSend(_val.Obj<FxSend>()))
    SYNC_PROP_SET(reverb_mix_db, GetReverbMixDb(), SetReverbMixDb(_val.Float()))
    SYNC_PROP_SET(reverb_enable, GetReverbEnable(), SetReverbEnable(_val.Int()))
    SYNC_PROP(faders, mFaders)
    SYNC_PROP(patchnum, mPatchNumber)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(MidiInstrument)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMultiSampleMap;
    bs << mSend;
    bs << mPatchNumber;
    mFaders.Save(bs);
    bs << mReverbMixDb;
    bs << mReverbEnable;
END_SAVES

BEGIN_COPYS(MidiInstrument)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MidiInstrument)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax)
            COPY_MEMBER(mMultiSampleMap)
        COPY_MEMBER(mSend)
        COPY_MEMBER(mPatchNumber)
        COPY_MEMBER(mReverbMixDb)
        COPY_MEMBER(mReverbEnable)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(MidiInstrument)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mMultiSampleMap;
    d >> mSend;
    d >> mPatchNumber;
    mFaders.Load(d.stream);
    if (d.rev >= 3) {
        d >> mReverbMixDb;
        d >> mReverbEnable;
    }
    StartPolling();
END_LOADS

NoteVoiceInst *MidiInstrument::MakeNoteInst(
    SampleZone *zone,
    unsigned char triggerNote,
    unsigned char ratio,
    int durFramesLeft,
    int glideID
) {
    NoteVoiceInst *inst = new NoteVoiceInst(
        this, zone, triggerNote, ratio, durFramesLeft, glideID, mFineTuneCents
    );
    inst->UpdateVolume();
    return inst;
}

void MidiInstrument::SetReverbMixDb(float db) {
    mReverbMixDb = db;
    FOREACH (it, mActiveVoices) {
        (*it)->Sample()->SetReverbMixDb(mReverbMixDb);
    }
}

void MidiInstrument::SetReverbEnable(bool enable) {
    mReverbEnable = enable;
    FOREACH (it, mActiveVoices) {
        (*it)->Sample()->SetReverbEnable(mReverbEnable);
    }
}

void MidiInstrument::SetSend(FxSend *send) {
    mSend = send;
    FOREACH (it, mActiveVoices) {
        (*it)->Sample()->SetSend(mSend);
    }
}

void MidiInstrument::StartSample(
    unsigned char note, unsigned char vel, int durFramesLeft, int glideID
) {
    for (int i = 0; i < mMultiSampleMap.size(); i++) {
        SampleZone &cur = mMultiSampleMap[i];
        if (cur.Includes(note, vel)) {
            NoteVoiceInst *inst = MakeNoteInst(&cur, note, vel, durFramesLeft, glideID);
            mActiveVoices.push_back(inst);
            inst->Start();
        }
    }
}
