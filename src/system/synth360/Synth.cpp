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
#include "synth360/HeadsetXferEffect.h"
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
#include "xdk/win_types.h"
#include "xdk/xaudio2/xapo.h"
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/xaudio2/xaudio2fx.h"

Synth360 *TheXboxSynth;

Synth360::Synth360()
    : unke8(0), mXAudio(0), mOutputVoice(0), mReverbVoice(0), mReverbSendVoice(0),
      unkfc(0), unk104(true), unk105(false), unk138(false), unk13c(0), unk14c(false) {}

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
    XAudio2Create(&mXAudio, 0, 0x3C);
    mXAudio->CreateMasteringVoice(&mOutputVoice, 0, 0, 0, 0, nullptr);

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

    if (mOutputVoice) {
        mOutputVoice->SetEffectChain(&effectChain);
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
    if (mOutputVoice) {
        mOutputVoice->GetEffectParameters(0, &p, sizeof(p));
    }
    p.thresholdDB = threshold;
    p.ratio = ratio;
    p.attack = attackMs;
    p.release = releaseMs;
    p.gateThresholdDB = -140;
    p.outputLevel = (1 - (1 / ratio)) * threshold + outputDb;
    if (mOutputVoice) {
        mOutputVoice->SetEffectParameters(0, &p, sizeof(p), 0);
    }
    if (mXAudio && mOutputVoice) {
        XAudio2CreateReverb(&mReverbAPO);
        effectChain.pEffectDescriptors = effectDescs;
        effectDescs[0].pEffect = mReverbAPO;
        effectDescs[0].InitialState = true;
        effectDescs[0].OutputChannels = 2;
        effectChain.EffectCount = 1;
        mXAudio->CreateSubmixVoice(
            &mReverbVoice, 2, 48000, 0, 0x8000, nullptr, &effectChain
        );
        // clang-format off
        {
            XAUDIO2_SEND_DESCRIPTOR desc = { 0, mReverbVoice };
            XAUDIO2_VOICE_SENDS sends = { 1, &desc };
            mXAudio->CreateSubmixVoice(&mReverbSendVoice, 6, 48000, 0, 0x7FFF, &sends, nullptr);
            mReverbVoice->SetVolume(4, 0);
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
    for (int i = 0; i < mFxSends.size(); i++) {
        mFxSends[i]->CleanChain();
    }
    TerminateVoiceThread();
    TheXboxSynth = nullptr;
    Synth::Terminate();
    ExternalMic::Terminate();
    std::for_each(mMics.begin(), mMics.end(), Delete());
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->Shutdown();
    }
    if (!mHeadsetSubmixes.empty()) {
        unke8->Stop(0, 0);
        unke8->DestroyVoice();
        unke8 = nullptr;
        for (int i = 0; i < mHeadsetSubmixes.size(); i++) {
            mHeadsetSubmixes[i]->DestroyVoice();
        }
        mHeadsetSubmixes.clear();
    }
    if (mReverbSendVoice) {
        mReverbSendVoice->DestroyVoice();
        mReverbSendVoice = nullptr;
    }
    if (mReverbVoice) {
        mReverbVoice->DestroyVoice();
        mReverbVoice = nullptr;
    }
    if (unkfc) {
        unkfc->DestroyVoice();
        unkfc = nullptr;
    }
    if (mOutputVoice) {
        mOutputVoice->DestroyVoice();
        mOutputVoice = nullptr;
    }
    if (mXAudio) {
        mXAudio->Release();
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
    if (mOutputVoice) {
        if (enable) {
            mOutputVoice->EnableEffect(0, 0);
        } else {
            mOutputVoice->DisableEffect(0, 0);
        }
    }
}

void Synth360::RequirePushToTalk(bool b, int i) {
    if (!mMics.empty()) {
        MicManagerXbox::GetInstance()->RequirePushToTalk(b, i);
    }
}

void Synth360::AddFxSend(FxSend360 *fx) { mFxSends.push_back(fx); }

void Synth360::RemoveFxSend(FxSend360 *fx) {
    auto *findFx = std::find(mFxSends.begin(), mFxSends.end(), fx);
    if (findFx != mFxSends.end()) {
        mFxSends.erase(findFx);
    }
}

IXAudio2SubmixVoice *Synth360::GetHeadsetSubmix(int i) {
    if (!mHeadsetSubmixes.empty() && i != -1) {
        return mHeadsetSubmixes[i];
    }
    return nullptr;
}

struct PresetConfig {
    Symbol name;
    XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset;
};

void Synth360::SetGlobalReverbPreset(const char *name) {
    static PresetConfig sConfigs[] = {
        { "default", XAUDIO2FX_I3DL2_PRESET_DEFAULT },
        { "generic", XAUDIO2FX_I3DL2_PRESET_GENERIC },
        { "padded_cell", XAUDIO2FX_I3DL2_PRESET_PADDEDCELL },
        { "room", XAUDIO2FX_I3DL2_PRESET_ROOM },
        { "bath_room", XAUDIO2FX_I3DL2_PRESET_BATHROOM },
        { "living_room", XAUDIO2FX_I3DL2_PRESET_LIVINGROOM },
        { "stone_room", XAUDIO2FX_I3DL2_PRESET_STONEROOM },
        { "auditorium", XAUDIO2FX_I3DL2_PRESET_AUDITORIUM },
        { "concert_hall", XAUDIO2FX_I3DL2_PRESET_CONCERTHALL },
        { "cave", XAUDIO2FX_I3DL2_PRESET_CAVE },
        { "arena", XAUDIO2FX_I3DL2_PRESET_ARENA },
        { "hangar", XAUDIO2FX_I3DL2_PRESET_HANGAR },
        { "carpeted_hallway", XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY },
        { "hallway", XAUDIO2FX_I3DL2_PRESET_HALLWAY },
        { "stone_corridor", XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR },
        { "alley", XAUDIO2FX_I3DL2_PRESET_ALLEY },
        { "forest", XAUDIO2FX_I3DL2_PRESET_FOREST },
        { "city", XAUDIO2FX_I3DL2_PRESET_CITY },
        { "mountains", XAUDIO2FX_I3DL2_PRESET_MOUNTAINS },
        { "quarry", XAUDIO2FX_I3DL2_PRESET_QUARRY },
        { "plain", XAUDIO2FX_I3DL2_PRESET_PLAIN },
        { "parking_lot", XAUDIO2FX_I3DL2_PRESET_PARKINGLOT },
        { "sewer_pipe", XAUDIO2FX_I3DL2_PRESET_SEWERPIPE },
        { "underwater", XAUDIO2FX_I3DL2_PRESET_UNDERWATER },
        { "small_room", XAUDIO2FX_I3DL2_PRESET_SMALLROOM },
        { "medium_room", XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM },
        { "large_room", XAUDIO2FX_I3DL2_PRESET_LARGEROOM },
        { "medium_hall", XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL },
        { "large_hall", XAUDIO2FX_I3DL2_PRESET_LARGEHALL },
        { "plate", XAUDIO2FX_I3DL2_PRESET_PLATE }
    };
    XAUDIO2FX_REVERB_PARAMETERS native;
    if (name && *name) {
        int idx = 0;
        for (; idx < DIM(sConfigs); idx++) {
            if (sConfigs[idx].name == name) {
                break;
            }
        }
        if (idx == DIM(sConfigs)) {
            MILO_FAIL("Unexpected environment preset.");
        }
        ReverbConvertI3DL2ToNative(&sConfigs[idx].preset, &native);
    } else {
        mReverbVoice->GetEffectParameters(0, &native, sizeof(native));
        native.DecayTime = 1.6f;
    }
    mReverbVoice->SetEffectParameters(0, &native, sizeof(native), 0);
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

void Synth360::SetupHeadsetSubmixes() {
    mHeadsetSubmixes.resize(4);
    for (int i = 0; i < 4; i++) {
        XAUDIO2_EFFECT_DESCRIPTOR desc;
        desc.pEffect = static_cast<CXAPOBase *>(new HeadsetXferEffect());
        desc.InitialState = false;
        desc.OutputChannels = 1;
        XAUDIO2_EFFECT_CHAIN chain;
        chain.EffectCount = 1;
        chain.pEffectDescriptors = &desc;
        mXAudio->CreateSubmixVoice(&mHeadsetSubmixes[i], 1, 48000, 0, 0, 0, &chain);
    }
    // clang-format off
    std::vector<XAUDIO2_SEND_DESCRIPTOR> audioDescs;
    XAUDIO2_SEND_DESCRIPTOR audioDesc;
    audioDesc.Flags = 0;
    for(int i = 0; i < 4; i++){
        audioDesc.pOutputVoice = mHeadsetSubmixes[i];
        audioDescs.push_back(audioDesc);
    }
    // clang-format on 
    IXAudio2SourceVoice* v;
    WAVEFORMATEX wav = { 1, 1, 48000, 96000, 2, 16, 0 };
    XAUDIO2_VOICE_SENDS sends = { audioDescs.size(), &audioDescs[0] };
    HRESULT hr = mXAudio->CreateSourceVoice(&v,&wav,2,2,nullptr,&sends,nullptr);
    MILO_ASSERT(SUCCEEDED(hr), 0x30A);

    static BYTE sAudioData[0x100];
    XAUDIO2_BUFFER buffer;
    buffer.Flags = 0;
    memset(&buffer.AudioBytes, 0, sizeof(buffer) - sizeof(buffer.Flags)); // terrible
    buffer.AudioBytes = sizeof(sAudioData);
    buffer.pAudioData = sAudioData;
    buffer.PlayBegin = 0;
    buffer.PlayLength = 0;
    buffer.LoopBegin = 0;
    buffer.LoopLength = 0;
    buffer.LoopCount = 255;
    buffer.pContext = nullptr;
    hr = unke8->SubmitSourceBuffer(&buffer, nullptr);
    MILO_ASSERT(SUCCEEDED(hr), 0x319);
    hr = unke8->Start(0, 0);
    MILO_ASSERT(SUCCEEDED(hr), 0x31C);
}

// defined in Synth360's Synth.cpp
Synth *Synth::New() { return new Synth360(); }
