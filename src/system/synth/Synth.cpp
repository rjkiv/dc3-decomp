#include "synth/Synth.h"
#include "AudioDucker.h"
#include "Emitter.h"
#include "FxSendBitCrush.h"
#include "FxSendChorus.h"
#include "FxSendCompress.h"
#include "FxSendDistortion.h"
#include "FxSendEQ.h"
#include "FxSendFlanger.h"
#include "FxSendMeterEffect.h"
#include "FxSendSynapse.h"
#include "FxSendWah.h"
#include "KeyChain.h"
#include "MeterEffectMonitor.h"
#include "MidiInstrument.h"
#include "MoggClip.h"
#include "Sound.h"
#include "ThreeDSound.h"
#include "Utl.h"
#include "flow/Flow.h"
#include "math/Color.h"
#include "math/Decibels.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/BufFile.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "os/System.h"
#include "rndobj/Overlay.h"
#include "rndobj/Rnd.h"
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth/FxSendDelay.h"
#include "synth/FxSendPitchShift.h"
#include "synth/FxSendReverb.h"
#include "synth/MicNull.h"
#include "synth/Pollable.h"
#include "synth/Sequence.h"
#include "synth/Sfx.h"
#include "synth/StreamNull.h"
#include "synth/SynthSample.h"
#include "synth/WavMgr.h"
#include "utl/Cache.h"
#include "utl/Loader.h"
#include <cstdio>

static const float sDrawMeterFloats[3] = { 0.2f, 40, 0.7f };

namespace {
    struct DebugGraph {
        DebugGraph(const Hmx::Color &c) {
            mVals.resize(200);
            mIdx = 0;
            mColor = c;
        }

        std::vector<float> mVals; // 0x0
        int mIdx; // 0xc
        Hmx::Color mColor; // 0x10
    };

    std::vector<DebugGraph> gDebugGraphs;
}

Loader *WavFactory(const FilePath &file, LoaderPos pos) {
    CacheResourceResult res;
    return new FileLoader(
        file, CacheWav(file.c_str(), res), pos, 0, false, true, nullptr, nullptr
    );
}

DataNode returnMasterKey(DataArray *msg) {
    unsigned char str[16];
    unsigned char masher[64];
    if (msg->Size() > 1) {
        KeyChain::getMasher(masher);
        // obfuscation moment, becomes "{Na}"
        str[0] = 'z';
        str[1] = 'M';
        str[2] = '`';
        str[3] = '|';
        str[4] = '\xFF';
        for (int i = 0; i < 5; i++) {
            str[i]++;
        }
        DataArray *data = DataReadString((char *)str);
        int i2 = data->Evaluate(0).Int();
        data->Release();
        int i3 = msg->Int(1);
        memcpy((void *)(i3 ^ i2), masher, 0x40);
    }
    return 0;
}

Synth *TheSynth;

Synth::Synth()
    : mTrackLevels(false), mMuted(false), mMicClientMapper(nullptr), unk98(0),
      mDebugStream(0), mADSR(nullptr) {
    SetName("synth", ObjectDir::Main());
    DataArray *cfg = SystemConfig("synth");
    cfg->FindData("mics", mNumMics, true);
    cfg->FindData("track_levels", mTrackLevels, false);
    mMidiSynth = new MidiSynth();
    gDebugGraphs.push_back(DebugGraph(Hmx::Color(1, 0, 0)));
    gDebugGraphs.push_back(DebugGraph(Hmx::Color(0, 1, 0)));
    gDebugGraphs.push_back(DebugGraph(Hmx::Color(1, 1, 0)));
    gDebugGraphs.push_back(DebugGraph(Hmx::Color(1, 1, 1)));
    mMicClientMapper = new MicClientMapper();
    MILO_ASSERT(!TheSynth, 0xC0);
    mADSR = new ADSRImpl();
}

BEGIN_HANDLERS(Synth)
    HANDLE(play, OnPassthrough)
    HANDLE(stop, OnPassthrough)
    HANDLE_ACTION(run_flow, RunFlow(_msg->Str(2)))
    HANDLE(start_mic, OnStartMic)
    HANDLE(stop_mic, OnStopMic)
    HANDLE_ACTION(stop_playback_all_mics, StopPlaybackAllMics())
    HANDLE(num_connected_mics, OnNumConnectedMics)
    HANDLE_EXPR(did_mics_change, DidMicsChange())
    HANDLE_ACTION(reset_mics_changed, ResetMicsChanged())
    HANDLE(set_mic_volume, OnSetMicVolume)
    HANDLE(set_fx, OnSetFX)
    HANDLE(set_fx_vol, OnSetFXVol)
    HANDLE_ACTION(stop_all_sfx, StopAllSfx(_msg->Size() == 3 ? _msg->Int(2) : false))
    HANDLE_ACTION(pause_all_sfx, PauseAllSfx(_msg->Int(2)))
    HANDLE_EXPR(master_vol, GetMasterVolume())
    HANDLE_ACTION(set_master_vol, SetMasterVolume(_msg->Float(2)))
    HANDLE_EXPR(find, Find<Hmx::Object>(_msg->Str(2), true))
    HANDLE_ACTION(toggle_hud, ToggleHud())
    HANDLE_EXPR(
        get_sample_mem, GetSampleMem(_msg->Obj<ObjectDir>(2), (Platform)_msg->Int(3))
    )
    HANDLE_EXPR(spu_overhead, GetSPUOverhead())
    HANDLE_ACTION(set_headset_target, 0)
    HANDLE_ACTION(stop_all_sounds, StopAllSounds())
    HANDLE_ACTION(set_vo_edit_sound, unka8 = _msg->Str(2))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void Synth::Init() {
    SynthUtlInit();
    REGISTER_OBJ_FACTORY(Fader);
    Sfx::Init();
    REGISTER_OBJ_FACTORY(MidiInstrument)
    SynthSample::Init();
    Sequence::Init();
    SynthEmitter::Init();
    REGISTER_OBJ_FACTORY(FxSendReverb)
    REGISTER_OBJ_FACTORY(FxSendDelay)
    REGISTER_OBJ_FACTORY(FxSendBitCrush)
    REGISTER_OBJ_FACTORY(FxSendDistortion)
    REGISTER_OBJ_FACTORY(FxSendCompress)
    REGISTER_OBJ_FACTORY(FxSendEQ)
    REGISTER_OBJ_FACTORY(FxSendFlanger)
    REGISTER_OBJ_FACTORY(FxSendChorus)
    REGISTER_OBJ_FACTORY(FxSendMeterEffect)
    REGISTER_OBJ_FACTORY(FxSendPitchShift)
    REGISTER_OBJ_FACTORY(FxSendSynapse)
    REGISTER_OBJ_FACTORY(FxSendWah)
    REGISTER_OBJ_FACTORY(MoggClip)
    REGISTER_OBJ_FACTORY(MeterEffectMonitor)
    REGISTER_OBJ_FACTORY(Sound)
    REGISTER_OBJ_FACTORY(ADSR)
    REGISTER_OBJ_FACTORY(ThreeDSound)
    REGISTER_OBJ_FACTORY(AudioDuckerTrigger)
    mMasterFader = Hmx::Object::New<Fader>();
    mSfxFader = Hmx::Object::New<Fader>();
    mMidiInstrumentFader = Hmx::Object::New<Fader>();
    DataArray *cfg = SystemConfig("synth");
    mMuted = cfg->FindInt("mute");
    TheLoadMgr.RegisterFactory("wav", WavFactory);
    mMics.resize(mNumMics);
    for (int i = 0; i < mMics.size(); i++) {
        mMics[i] = new MicNull();
    }
    mHud = RndOverlay::Find("synth_hud", true);
    mHud->SetCallback(this);
    InitSecurity();
}

void Synth::InitSecurity() {
    char buf[256];
    buf[1] = '\0';
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            buf[0] = j + ('A' + i * 4);
            DataRegisterFunc(buf, returnMasterKey);
        }
    }
    buf[0] = 'M';
    DataRegisterFunc(buf, returnMasterKey);
    mByteGrinder.Init();
}

void Synth::Terminate() {
    MILO_ASSERT(mZombieInsts.empty(), 0x116);
    DeleteAll(mMics);
    RELEASE(mMidiSynth);
    RELEASE(mMasterFader);
    RELEASE(mSfxFader);
    RELEASE(mMidiInstrumentFader);
    RELEASE(mMicClientMapper);
    SynthUtlTerm();
}

void Synth::Poll() {
    for (int i = 0; i < mLevelData.size(); i++) {
        LevelData &data = mLevelData[i];
        if ((data.mPeak > data.mPeakHold) || ++data.mPeakAge >= 0x3C) {
            data.mPeakHold = data.mPeak;
            data.mPeakAge = 0;
        }
    }
    if (mMuted)
        mMasterFader->SetVolume(-96.0f);
    SynthPollable::PollAll();
    if (DidMicsChange()) {
        MILO_ASSERT(mMicClientMapper, 0x14E);
        mMicClientMapper->HandleMicsChanged();
        ResetMicsChanged();
    }
    if (!mZombieInsts.empty()) {
        CullZombies();
    }
}

Stream *Synth::NewStream(const char *, float f1, float, bool) {
    return new StreamNull(f1);
}

Stream *Synth::NewBufStream(const void *, int, Symbol, float f1, bool) {
    return new StreamNull(f1);
}

void Synth::NewStreamFile(const char *fileBase, File *&fileOut, Symbol &extOut) {
    static char gFakeFile[16];
    fileOut = new BufFile(gFakeFile, sizeof(gFakeFile));
    extOut = "fake";
}

FxSendPitchShift *Synth::CreatePitchShift(int stage, SendChannels channels) {
    FxSendPitchShift *pitchShift = Hmx::Object::New<FxSendPitchShift>();
    pitchShift->SetStage(stage);
    pitchShift->SetChannels(channels);
    return pitchShift;
}

void Synth::DestroyPitchShift(FxSendPitchShift *shift) { delete shift; }

float Synth::UpdateOverlay(RndOverlay *o, float y) {
    Hmx::Color white(1, 1, 1);
    float f24 = (float)TheRnd.Height() * (y + 0.265f);
    if (mDebugStream) {
        DrawMeterScale(f24);
        float volume = mDebugStream->Faders()->GetVolume();
        DrawMeter(f24, volume, 0, "stream");
        for (int i = 0; i < mDebugStream->GetNumChannels(); i++) {
            DrawMeter(
                f24,
                mDebugStream->ChannelFaders(i).GetVolume(),
                0,
                MakeString("chan %i", i)
            );
        }
    }
    if (!mLevelData.empty()) {
        DrawMeterScale(f24);
    }
    for (int i = 0; i < mLevelData.size(); i++) {
        float rms = RatioToDb(mLevelData[i].mRMS);
        float peakhold = RatioToDb(mLevelData[i].mPeakHold);
        if (rms > 2) {
            rms = -30;
        }
        DrawMeter(f24, rms, peakhold, mLevelData[i].mName.c_str());
    }
    auto pollIt = SynthPollable::Pollables().begin();
    char buf[64];
    sprintf(buf, "Total active Sequences: %d", SynthPollable::Pollables().size());
    TheRnd.DrawString(buf, Vector2(100, f24), white, true);
    float f12 = f24 + 12.0f;
    for (; pollIt != SynthPollable::Pollables().end(); ++pollIt) {
        const char *name = (*pollIt)->GetSoundDisplayName();
        if (*name != '\0') {
            TheRnd.DrawString(name, Vector2(100, f12), white, true);
            f12 += 12.0f;
        }
    }
    return f12 / (float)TheRnd.Height();
}

void Synth::SetMasterVolume(float volume) { mMasterFader->SetVolume(volume); }

float Synth::GetMasterVolume() { return mMasterFader->DuckedValue(); }

void Synth::ToggleHud() {
    mHud->SetShowing(!mHud->Showing());
    if (!mTrackLevels) {
        EnableLevels(mHud->Showing());
    }
}

const ADSRImpl *Synth::DefaultADSR() {
    MILO_ASSERT(mADSR, 0x498);
    return mADSR;
}

void Synth::SetFX(const DataArray *data) {
    MILO_ASSERT(data, 0x165);
    SetFXChain(data->FindInt("chain"));
    for (int i = 0; i < 2; i++) {
        DataArray *coreArr = data->FindArray(MakeString("core_%i", i));
        int mode = coreArr->FindArray("mode")->Int(1);
        float volume = coreArr->FindArray("volume")->Float(1);
        float delay = coreArr->FindArray("delay")->Float(1);
        float feedback = coreArr->FindArray("feedback")->Float(1);
        SetFXMode(i, (FXMode)mode);
        SetFXVolume(i, volume);
        SetFXDelay(i, delay);
        SetFXFeedback(i, feedback);
    }
}

void Synth::SetMic(const DataArray *data) {
    for (int i = 0; i < mNumMics; i++) {
        Mic *mic = GetMic(i);
        if (mic)
            mic->Set(data);
    }
    SetMicFX(data->FindInt("fx"));
    SetMicVolume(data->FindFloat("volume"));
}

bool Synth::CheckCommonBank(bool notify) {
    bool loaded = unk64 && unk64.IsLoaded();
    if (!loaded && notify) {
        MILO_LOG("Synth::Find() - Common sound bank not loaded!\n");
    }
    return loaded;
}

int Synth::GetFXOverhead() {
    int overheads[10] = { 0x80,   0x26c0, 8000,    0x4c28,  0x6fe0,
                          0xade0, 0xf6c0, 0x18040, 0x18040, 0x3c00 };
    DataArray *cfg = SystemConfig("synth");
    int mode = cfg->FindArray("fx")->FindArray("core_0")->FindInt("mode");
    return overheads[mode] + 0x20000;
}

int Synth::GetSPUOverhead() {
    DataArray *cfg = SystemConfig("synth");
    int spuBufs = cfg->FindArray("iop")->FindInt("spu_buffers");
    spuBufs *= 0x800;
    spuBufs += 0x5010;
    return spuBufs + GetFXOverhead();
}

void Synth::StopPlaybackAllMics() {
    if (mMicClientMapper->GetMicMgrInterface()) {
        mMicClientMapper->GetMicMgrInterface()->SetPlayback(false);
    }
}

void Synth::AddPlayHandler(Hmx::Object *obj) { mPlayHandlers.push_back(obj); }
void Synth::RemovePlayHandler(Hmx::Object *obj) { mPlayHandlers.remove(obj); }

void Synth::SendToPlayHandlers(Sound *sound) {
    SoundPlayMsg msg(sound);
    FOREACH (it, mPlayHandlers) {
        (*it)->Handle(msg, false);
    }
}

void Synth::RunFlow(const char *flowName) {
    if (CheckCommonBank(false)) {
        Flow *flow = Find<Flow>(flowName, false);
        if (flow) {
            flow->Activate();
        } else {
            MILO_NOTIFY(
                "Synth::RunFlow() - %s not found in %s", flowName, unk64->GetPathName()
            );
        }
    }
}

void Synth::StopAllSfx(bool stop) {
    FOREACH (it, SynthPollable::Pollables()) {
        Sequence *seq = dynamic_cast<Sequence *>(*it);
        if (seq) {
            seq->Stop(stop);
        }
    }
}

void Synth::PauseAllSfx(bool pause) {
    FOREACH (it, SynthPollable::Pollables()) {
        Sfx *sfx = dynamic_cast<Sfx *>(*it);
        if (sfx) {
            sfx->Pause(pause);
        }
        Sound *sound = dynamic_cast<Sound *>(*it);
        if (sound) {
            sound->Pause(pause);
        }
    }
}

void Synth::PlaySound(const char *name, float volume, float pan, float transpose) {
    if (CheckCommonBank(false)) {
        Sound *sound = Find<Sound>(name, false);
        if (sound) {
            sound->Play(volume, pan, transpose, nullptr, 0);
        } else {
            MILO_NOTIFY(
                "Synth::PlaySound() - Sound %s not found in %s",
                name,
                unk64->GetPathName()
            );
        }
    }
}

void Synth::StopAllSounds() {
    FOREACH (it, SynthPollable::Pollables()) {
        Sound *sound = dynamic_cast<Sound *>(*it);
        if (sound) {
            sound->Stop(nullptr, true);
        }
    }
}

int Synth::GetSampleMem(ObjectDir *dir, Platform p) {
    int num = 0;
    for (ObjDirItr<SynthSample> it(dir, true); it != nullptr; ++it) {
        num += it->GetPlatformSize(p);
    }
    return num;
}

void Synth::AddZombie(SampleInst *inst) {
    inst->Stop(false);
    mZombieInsts.push_back(inst);
}

void Synth::CullZombies() {
    for (auto it = mZombieInsts.begin(); it != mZombieInsts.end();) {
        auto next = it++;
        if ((*it)->DonePlaying()) {
            mZombieInsts.erase(it);
        }
        it = next;
    }
}

void Synth::DrawMeterScale(float &y0) {
    float x1 = TheRnd.Width() * sDrawMeterFloats[0];
    float x2 = TheRnd.Width() * sDrawMeterFloats[2];
    Hmx::Color white(1, 1, 1);
    TheRnd.DrawString(MakeString("%i", -0x28), Vector2(x1, y0), white, true);
    TheRnd.DrawString(MakeString("%i", -0x14), Vector2(x2 / 2 + x1, y0), white, true);
    TheRnd.DrawString("0", Vector2(x2 + x1, y0), white, true);
    y0 += 16;
}

DataNode Synth::OnPassthrough(DataArray *a) {
    if (!CheckCommonBank(false))
        return 0;
    else {
        const char *name = a->Str(2);
        Hmx::Object *obj = Find<Hmx::Object>(name, false);
        if (obj)
            obj->Handle(a, true);
        else
            MILO_NOTIFY(
                "Synth::OnPassthrough() - %s not found in %s", name, unk64->GetPathName()
            );
        return 0;
    }
}

DataNode Synth::OnStartMic(const DataArray *a) {
    GetMic(a->Int(2))->Start();
    return 0;
}

DataNode Synth::OnStopMic(const DataArray *a) {
    GetMic(a->Int(2))->Stop();
    return 0;
}

DataNode Synth::OnNumConnectedMics(const DataArray *) { return GetNumConnectedMics(); }

DataNode Synth::OnSetMicVolume(const DataArray *a) {
    SetMicVolume(a->Float(2));
    return 0;
}

DataNode Synth::OnSetFX(const DataArray *a) {
    SetFX(a->Array(2));
    return 0;
}

DataNode Synth::OnSetFXVol(const DataArray *a) {
    SetFXVolume(a->Int(2), a->Float(3));
    return 0;
}

void SynthPreInit() {
    MILO_ASSERT(!TheSynth, 0x283);
    DataArray *cfg = SystemConfig("synth");
    bool useNullSynth = cfg->FindInt("use_null_synth");
    if (useNullSynth) {
        TheSynth = new Synth();
    } else {
        TheSynth = Synth::New();
    }
    if (TheSynth->Fail()) {
        RELEASE(TheSynth);
        TheSynth = new Synth();
    }
    TheSynth->PreInit();
    InitWavMgr();
}

void SynthInit() {
    if (!TheSynth)
        SynthPreInit();
    DataArray *cfg = SystemConfig("synth");
    TheSynth->Init();
    TheSynth->SetMic(cfg->FindArray("mic"));
    TheSynth->SetFX(cfg->FindArray("fx"));
    TheSynth->MasterFader()->SetVolume(cfg->FindFloat("master_vol"));
    TheDebug.AddExitCallback(SynthTerminate);
    PreloadSharedSubdirs("synth");
}

void SynthTerminate() {
    TheSynth->StopAllSounds();
    TheSynth->Poll();
    TheDebug.RemoveExitCallback(SynthTerminate);
    TheSynth->Terminate();
    RELEASE(TheSynth);
}
