#include "hamobj/HamMaster.h"
#include "HamAudio.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamSongData.h"
#include "midi/DataEventList.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth/Faders.h"
#include "synth/Synth.h"
#include "utl/Loader.h"
#include "utl/SongPos.h"
#include "utl/TimeConversion.h"

HamMaster::HamMaster(HamSongData *data, MidiParserMgr *mgr)
    : mSongData(data), mAudio(nullptr), mMidiParserMgr(mgr), mSongInfo(nullptr),
      mLoader(0), unk45(0), unk48(0), mStreamMs(-1), unk50(0), unk54(-1), unk58(-1),
      unk5c(-1), unk9c(0), unka0(0), unka4(0), unkb0(0), mMetronome(0) {
    Reset();
    mAudio = new HamAudio();
}

HamMaster::~HamMaster() { delete mAudio; }

BEGIN_HANDLERS(HamMaster)
    HANDLE_EXPR(stream_time, mStreamMs / 1000)
    HANDLE_EXPR(song_duration_ms, SongDurationMs())
    HANDLE_EXPR(event_beat, EventBeat(_msg->Sym(2)))
    HANDLE_ACTION(toggle_metronome, mMetronome = !mMetronome)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamMaster)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HamMaster::Poll(float f1) {
    if (IsLoaded() && mAudio->GetSongStream()) {
        unk48 = f1;
        unk60 = mSongData->CalcSongPos(this, unk48);
        float f8 = mAudio->GetSongStream()->GetJumpBackTotalTime(f1);
        float f9 = f8 + unk48;
        unk50 = f9 < mStreamMs;
        Marker marker1, marker2;
        bool jp = mAudio->GetSongStream()->CurrentJumpPoints(marker1, marker2);
        if (!unk50 && jp && marker1.posMS <= marker2.posMS) {
            if (mStreamMs <= marker2.posMS && marker2.posMS < f9) {
                unk50 = true;
            } else {
                unk50 = false;
            }
        }
        if (unk50) {
            float f10;
            if (jp) {
                f10 = MsToTick(marker2.posMS) - 1.0f;
            } else {
                f10 = unk60.GetTotalTick();
            }
            if (mMidiParserMgr) {
                mMidiParserMgr->Reset();
            }
            unk5c = mStreamMs;
            static Message msg("stream_jump");
            Export(msg, true);
        }
        CheckBeat();
        CheckLevels();
        mAudio->Poll();
    }
}

void HamMaster::Jump(float f1) {
    unk60 = mSongData->CalcSongPos(this, f1);
    const SongPos &tmp = unk60;
    unk78 = tmp;
    unkb4 = -1;
    unkb8 = 0;
    if (mMidiParserMgr) {
        mMidiParserMgr->Reset(tmp.GetTotalTick());
    }
    mAudio->Jump(f1);
}

void HamMaster::Reset() {
    unk78 = SongPos();
    unkb8 = 0;
    unkb4 = -1;
    for (int i = 0; i < unk90.size(); i++) {
        unk90[i] = 0;
    }
    if (mMidiParserMgr) {
        mMidiParserMgr->Reset();
    }
    static Message msg("first_beat");
    Export(msg, true);
    ResetAudio();
    mStreamMs = 0;
    unk54 = 0;
    unk50 = false;
    unk48 = 0;
    if (TheSynth->CheckCommonBank(false)) {
        Fader *fade = TheSynth->Find<Fader>("music_level.fade", false);
        if (fade) {
            AddMusicFader(fade);
        }
        Fader *crossfade = TheSynth->Find<Fader>("music_level_cross_fade.fade", false);
        if (crossfade) {
            AddMusicFader(crossfade);
        }
        Fader *fadeinout = TheSynth->Find<Fader>("skills_music_fade_in_out.fade", false);
        if (fadeinout) {
            AddMusicFader(fadeinout);
        }
    }
}

float HamMaster::SongDurationMs() {
    if (mMidiParserMgr) {
        DataEventList *events = mMidiParserMgr->GetEventsList();
        static Symbol end("end");
        for (int i = 0; i < events->Size(); i++) {
            const DataEvent &ev = events->Event(i);
            if (ev.Type(1) == end) {
                float tick = mSongData->GetBeatMap()->BeatToTick(ev.Start());
                return mSongData->GetTempoMap()->TickToTime(tick);
            }
        }
    }
    return 0;
}

void HamMaster::Load(
    SongInfo *s,
    bool b2,
    int i3,
    bool b4,
    HamSongDataValidate v,
    std::vector<MidiReceiver *> *
) {
    unk44 = b2;
    mSongInfo = s;
    mSongData->Load(s, b4, v);
    MILO_ASSERT(!mLoader, 0x69);
    mLoader = new HamMasterLoader(this);
    unk45 = false;
    if (b4) {
        TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
    }
}

void HamMaster::LoadOnlySongData(SongInfo *s, bool b, HamSongDataValidate v) {
    mSongInfo = s;
    mSongData->Load(s, b, v);
}

void HamMaster::ResetAudio() {
    if (mAudio && mAudio->GetTime()) {
        mAudio->Jump(0);
    }
}

float HamMaster::StreamMs() const { return mStreamMs; }

bool HamMaster::DetectStreamJump(float &f1, float &f2, float &f3) const {
    if (!unk50) {
        return false;
    } else {
        f1 = unk54;
        f2 = unk58;
        f3 = unk5c;
        return true;
    }
}

void HamMaster::AddMusicFader(Fader *fader) {
    if (GetAudio() && GetAudio()->GetSongStream()) {
        GetAudio()->GetSongStream()->Faders()->Add(fader);
    }
}

void HamMaster::SetMaps() { mSongData->SetMaps(); }

float HamMaster::EventBeat(Symbol s) {
    if (mMidiParserMgr) {
        DataEventList *events = mMidiParserMgr->GetEventsList();
        for (int i = 0; i < events->Size(); i++) {
            const DataEvent &ev = events->Event(i);
            if (ev.Type(1) == s) {
                return ev.Start();
            }
        }
    }
    return -1;
}

void HamMaster::LoaderPoll() {
    if (mSongData->Poll()) {
        if (TheLoadMgr.EditMode() && !TheSynth->CheckCommonBank(false)) {
            ObjDirPtr<ObjectDir> dir;
            DataArray *cfg = SystemConfig("sound", "banks", "common");
            dir.LoadFile(cfg->Str(1), false, true, kLoadFront, false);
            TheSynth->SetDir(dir);
        }
        mAudio->Load(mSongInfo, unk44);
        mSongInfo = nullptr;
        if (mMidiParserMgr) {
            mMidiParserMgr->FinishLoad();
        }
        unk45 = true;
        RELEASE(mLoader);
    }
}

void HamMaster::CheckBeat() {
    int totalbeat1 = unk60.GetTotalBeat();
    int totalbeat2 = unk78.GetTotalBeat();
    if (totalbeat1 != totalbeat2) {
        int beat = unk60.GetBeat();
        TheHamProvider->SetProperty("beat", beat + 1);
        if (mMetronome) {
            if (beat == 0) {
                TheSynth->PlaySound("metronome_measure", 0, 0, 0);
            } else {
                TheSynth->PlaySound("metronome_beat", 0, 0, 0);
            }
        }
        static DataNode &n = DataVariable("beat");
        n = totalbeat2;
        static Message msg("beat");
        Export(msg, true);
    }
    if (unk78.GetMeasure() != unk60.GetMeasure()) {
        static DataNode &n = DataVariable("measure");
        n = unk60.GetMeasure();
        static Message msg("downbeat");
        TheHamProvider->Export(msg, true);
    }
    if (unk78.GetTick() / 240 != unk60.GetTick() / 240) {
        static Message msg("halfbeat");
        TheHamProvider->Export(msg, true);
    }
    if (unk78.GetTick() / 120 != unk60.GetTick() / 120) {
        static Message msg("quarterbeat");
        TheHamProvider->Export(msg, true);
    }
    unk78 = unk60;
}

HamMasterLoader::HamMasterLoader(HamMaster *master)
    : Loader("", kLoadBack), mMaster(master) {}

void HamMasterLoader::PollLoading() { mMaster->LoaderPoll(); }
