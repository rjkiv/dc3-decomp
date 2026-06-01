#include "synth360/Synth.h"
#include "FxSendBitCrush.h"
#include "FxSendChorus.h"
#include "FxSendCompress.h"
#include "FxSendDelay.h"
#include "FxSendDistortion.h"
#include "FxSendEQ.h"
#include "FxSendFlanger.h"
#include "FxSendMeterEffect.h"
#include "FxSendReverb.h"
#include "FxSendWah.h"
#include "Synth.h"
#include "dsp/CompressionEffect.h"
#include "dsp/StandardEffect.h"
#include "macros.h"
#include "math/Decibels.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/BufFile.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "synth/BinkReader.h"
#include "synth/Mic.h"
#include "synth/StandardStream.h"
#include "synth/StreamNull.h"
#include "synth/StreamReader.h"
#include "synth/Synth.h"
#include "synth/VorbisReader.h"
#include "synth/WavReader.h"
#include "synth360/ExternalMic.h"
#include "synth360/FxSend.h"
#include "synth360/FxSendPitchShift.h"
#include "synth360/FxSendSynapse.h"
#include "synth360/MeterEffect.h"
#include "synth360/Mic.h"
#include "synth360/StreamReceiver.h"
#include "synth360/SynthSample.h"
#include "synth360/Voice.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"
#include "xdk/XAUDIO2.h"
#include "xdk/XBOXKRNL.h"

Synth360 *TheXboxSynth;

Synth360::Synth360()
    : unke8(0), unkec(0), unkf0(0), unkf4(0), unkf8(0), unkfc(0), unk104(true),
      unk105(false), unk138(false), unk13c(0), unk14c(false) {}

BEGIN_HANDLERS(Synth360)
    HANDLE_ACTION(set_headset_target, Voice::sHeadsetTarget = _msg->Int(2))
    HANDLE_SUPERCLASS(Synth)
END_HANDLERS

void Synth360::PreInit() {
    TheXboxSynth = this;
    const char *levelNames[] = { "  FL", "  FR", "   C", " LFE", "  SL",
                                 "  SR", "",     " DML", " DMR" };
    for (int i = 0; i < DIM(levelNames); i++) {
        mLevelData.push_back(LevelData(levelNames[i]));
    }
    // using processors 3, 4, 5 and 6 for xaudio2
    XAudio2Create(&unkec, 0, 0x3C);
    unkec->CreateMasteringVoice(&unkf0, 0, 0, 0, 0, nullptr);

    XAUDIO2_EFFECT_DESCRIPTOR effectDescs[2];
    effectDescs[0].pEffect =
        static_cast<CXAPOBase *>(new StandardEffect<CompressionEffect>());
    effectDescs[0].InitialState = true;
    effectDescs[0].OutputChannels = 6;
    effectDescs[1].pEffect = static_cast<CXAPOBase *>(new MeterEffect());
    effectDescs[1].InitialState = true;
    effectDescs[1].OutputChannels = 6;

    unk13c = new LevelData *; // new on some 4 sized struct
    *unk13c = &mLevelData[0];

    XAUDIO2_EFFECT_CHAIN effectChain = { 2, effectDescs };

    if (unkf0) {
        unkf0->SetEffectChain(&effectChain);
    }

    DataArray *limiterCfg = SystemConfig("synth", "limiter");
    float threshold = limiterCfg->FindFloat("threshold");
    float ratio = limiterCfg->FindFloat("ratio");
    float attackMs = limiterCfg->FindFloat("attack_ms") / 1000;
    float releaseMs = limiterCfg->FindFloat("release_ms") / 1000;
    float outputDb = limiterCfg->FindFloat("output_db");

    CompressionEffect::Params p;
    p.thresholdDB = -6;
    p.ratio = 1;
    p.outputLevel = 1;
    p.attack = 0.005f;
    p.release = 0.2f;
    p.expRatio = 1;
    p.expAttack = 0.99f;
    p.expRelease = 1.01f;
    p.gateThresholdDB = -40;
    if (unkf0) {
        unkf0->GetEffectParameters(0, &p, sizeof(p));
    }
    p.thresholdDB = threshold;
    p.ratio = ratio;
    p.attack = attackMs;
    p.release = releaseMs;
    p.gateThresholdDB = -140;
    p.outputLevel = (1 - (1 / ratio)) * threshold + outputDb;
    if (unkf0) {
        unkf0->SetEffectParameters(0, &p, sizeof(p), 0);
    }
    if (unkec && unkf0) {
        XAudio2CreateReverb(&unk100);
        effectChain.pEffectDescriptors = effectDescs;
        effectDescs[0].pEffect = unk100;
        effectDescs[0].InitialState = true;
        effectDescs[0].OutputChannels = 2;
        effectChain.EffectCount = 1;
        unkec->CreateSubmixVoice(&unkf4, 2, 48000, 0, 0x8000, nullptr, &effectChain);
        // clang-format off
        {
            XAUDIO2_SEND_DESCRIPTOR desc = { 0, unkf4 };
            XAUDIO2_VOICE_SENDS sends = { 1, &desc };
            unkec->CreateSubmixVoice(&unkf8, 6, 48000, 0, 0x7FFF, &sends, nullptr);
            unkf4->SetVolume(4, 0);
        }
        String env;
        DataArray *cfg = SystemConfig("synth");
        cfg->FindData("reverb_environment", env, false);
        SetGlobalReverbPreset(env.c_str());
    }
    EnableLevels(TrackLevels());
// clang-format on
}

void Synth360::Init() {
    Synth::Init();
    SynthSample360::Init();
    StreamReceiver360::Init();
    REGISTER_OBJ_FACTORY(FxSendReverb360)
    REGISTER_OBJ_FACTORY(FxSendDelay360)
    REGISTER_OBJ_FACTORY(FxSendCompress360)
    REGISTER_OBJ_FACTORY(FxSendEQ360)
    REGISTER_OBJ_FACTORY(FxSendFlanger360)
    REGISTER_OBJ_FACTORY(FxSendMeterEffect360)
    REGISTER_OBJ_FACTORY(FxSendWah360)
    REGISTER_OBJ_FACTORY(FxSendBitCrush360)
    REGISTER_OBJ_FACTORY(FxSendDistortion360)
    REGISTER_OBJ_FACTORY(FxSendChorus360)
    REGISTER_OBJ_FACTORY(FxSendPitchShift360)
    REGISTER_OBJ_FACTORY(FxSendSynapse360)
    if (SystemConfig("synth")->FindInt("enable_headset_output")) {
        SetupHeadsetSubmixes();
    }
    float volume = 0;
    SystemConfig("synth", "mic")->FindData("volume", volume, false);
    if (GetNumMics() > 0) {
        MicManagerXbox::GetInstance()->Init();
        mMics.resize(GetNumMics());
        ExternalMic::Init();
        for (int i = 0; i < mMics.size(); i++) {
            mMics[i] = new MicXbox(-1, DbToRatio(volume));
            ExternalMicClientMgr::Associate(i, dynamic_cast<MicXbox *>(mMics[i]));
        }
    }
}

void Synth360::SetDolby(bool b1, bool b2) {
    if (b2) {
        unk104 = b1;
        UpdateDolby();
    } else if (unk104 != b1) {
        unk108.Restart();
        unk104 = b1;
        unk105 = true;
    }
}

bool Synth360::IsUsingDolby() const {
    DWORD cfg;
    XAudioGetSpeakerConfig(&cfg);
    return cfg & DolbyDigital;
}

void Synth360::Terminate() {
    for (int i = 0; i < unk140.size(); i++) {
        unk140[i]->CleanChain();
    }
    TerminateVoiceThread();
    TheXboxSynth = nullptr;
    Synth::Terminate();
    ExternalMic::Terminate();
    std::for_each(mMics.begin(), mMics.end(), Delete());
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->Shutdown();
    }
    if (!unkdc.empty()) {
        unke8->Stop(0, 0);
        unke8->DestroyVoice();
        unke8 = nullptr;
        for (int i = 0; i < unkdc.size(); i++) {
            unkdc[i]->DestroyVoice();
        }
        unkdc.clear();
    }
    if (unkf8) {
        unkf8->DestroyVoice();
        unkf8 = nullptr;
    }
    if (unkf4) {
        unkf4->DestroyVoice();
        unkf4 = nullptr;
    }
    if (unkfc) {
        unkfc->DestroyVoice();
        unkfc = nullptr;
    }
    if (unkf0) {
        unkf0->DestroyVoice();
        unkf0 = nullptr;
    }
    if (unkec) {
        unkec->Release();
    }
    RELEASE(unk13c);
}

bool Synth360::HasPendingVoices() { return Voice::HasPendingVoices(); }

Mic *Synth360::GetMic(int index) { return mMics[index]; }
int Synth360::GetNumConnectedMics() { return ExternalMic::NumConnectedMics(); }

int Synth360::GetNextAvailableMicID() const {
    for (int i = 0; i < mMics.size(); i++) {
        if (!mMics[i]->IsInUse() && mMics[i]->GetType() != 0)
            return i;
    }
    return -1;
}

bool Synth360::IsMicConnected(int i) const {
    if (i >= 0 && i < mMics.size()) {
        return mMics[i]->GetType() != Mic::kDisconnected;
    } else {
        return false;
    }
}

void Synth360::CaptureMic(int micID) {
    MILO_ASSERT_RANGE(micID, 0, mMics.size(), 0x350);
    MILO_ASSERT(!mMics[micID]->IsInUse(), 0x351);
    mMics[micID]->MarkAsInUse(true);
}

void Synth360::ReleaseMic(int micID) {
    MILO_ASSERT_RANGE(micID, 0, mMics.size(), 0x35b);
    if (!mMics[micID]->IsInUse()) {
        MILO_NOTIFY_ONCE("Releasing a microphone [%d]that was not in use\n", micID);
    }
    mMics[micID]->MarkAsInUse(false);
}

void Synth360::ReleaseAllMics() {
    for (int i = 0; i < mMics.size(); i++) {
        mMics[i]->MarkAsInUse(false);
    }
}

bool Synth360::DidMicsChange() const {
    if (mMics.empty())
        return false;
    else {
        return MicManagerXbox::GetInstance()->MicsChanged();
    }
}

void Synth360::ResetMicsChanged() {
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->ClearMicsChanged();
    }
}

Stream *Synth360::NewStream(const char *cc, float f2, float f3, bool b4) {
    File *file;
    Symbol sym;
    NewStreamFile(cc, file, sym);
    if (file) {
        return new StandardStream(file, f2, f3, sym, b4, true, false);
    } else {
        MILO_NOTIFY("couldn't find stream %s", cc);
        return new StreamNull(f2);
    }
}

Stream *Synth360::NewBufStream(const void *v, int i2, Symbol s, float f4, bool b5) {
    return new StandardStream(new BufFile(v, i2), f4, 0, s, false, b5, false);
}

StreamReader *Synth360::NewStreamDecoder(File *f, StandardStream *ss, Symbol type) {
    if (type == "bik") {
        return new BinkReader(f, ss);
    } else if (type == "mogg") {
        return new VorbisReader(f, true, ss, true);
    } else if (type == "wav") {
        return new WavReader(f, ss);
    } else {
        MILO_FAIL("bad decoder type: %s", type);
        return nullptr;
    }
}

void Synth360::NewStreamFile(const char *cc, File *&file, Symbol &type) {
    String bikStr(MakeString("%s.bik", cc));
    String moggStr(MakeString("%s.mogg", cc));
    String wavStr(MakeString("%s.wav", cc));
    file = NewFile(bikStr.c_str(), FILE_OPEN_READ);
    if (file) {
        static Symbol bik("bik");
        type = bik;
    } else {
        file = NewFile(moggStr.c_str(), FILE_OPEN_READ);
        if (file) {
            static Symbol mogg("mogg");
            type = mogg;
        } else {
            file = NewFile(wavStr.c_str(), FILE_OPEN_READ);
            if (file) {
                static Symbol wav("wav");
                type = wav;
            } else {
                Synth::NewStreamFile(cc, file, type);
            }
        }
    }
}

void Synth360::EnableLevels(bool enable) {
    if (unkf0) {
        if (enable) {
            unkf0->EnableEffect(0, 0);
        } else {
            unkf0->DisableEffect(0, 0);
        }
    }
}

void Synth360::RequirePushToTalk(bool b, int i) {
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->RequirePushToTalk(b, i);
    }
}

void Synth360::AddFxSend(FxSend360 *fx) { unk140.push_back(fx); }

void Synth360::RemoveFxSend(FxSend360 *fx) {
    auto *findFx = std::find(unk140.begin(), unk140.end(), fx);
    if (findFx != unk140.end()) {
        unk140.erase(findFx);
    }
}

IXAudio2SubmixVoice *Synth360::GetHeadsetSubmix(int i) {
    if (!unkdc.empty() && i != -1) {
        return unkdc[i];
    }
    return nullptr;
}

void Synth360::UpdateDolby() {
    DWORD cfg;
    XAudioGetSpeakerConfig(&cfg);
    XCONFIG_USER_AUDIO_FLAGS flag = DolbyDigital;
    if (!unk104) {
        flag = LowLatency;
    }
    if ((cfg & flag) != flag) {
        XAudioOverrideSpeakerConfig(flag);
    }
}

// defined in Synth360's Synth.cpp
Synth *Synth::New() { return new Synth360(); }
