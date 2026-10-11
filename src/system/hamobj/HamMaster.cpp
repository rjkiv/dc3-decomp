#include "hamobj/HamMaster.h"
#include "HamAudio.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamSongData.h"
#include "math/Decibels.h"
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

HamMaster *TheMaster;

HamMaster::HamMaster(HamSongData *data, MidiParserMgr *mgr)
    : mSongData(data), mAudio(nullptr), mMidiParserMgr(mgr), mSongInfo(nullptr),
      mLoader(0), mLoaded(0), mLastStreamMs(0), mStreamMs(-1), mLoopBack(0),
      mLoopBackFromStreamMs(-1), mLastLoopFromMs(-1), mLastLoopToMs(-1),
      mLevelHighWaterMark(0), mActivePlayerTrack(0), mMetronome(0) {
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

void HamMaster::Poll(float ms) {
    if (IsLoaded() && mAudio->GetSongStream()) {
        mLastStreamMs = ms;
        mSongPos = mSongData->CalcSongPos(this, mLastStreamMs);
        float f9 =
            mAudio->GetSongStream()->GetJumpBackTotalTime(mLastStreamMs) + mLastStreamMs;
        mLoopBack = f9 < mStreamMs;
        Marker marker2, marker1;
        bool jp = mAudio->GetSongStream()->CurrentJumpPoints(marker1, marker2);
        if (!mLoopBack && jp && marker1.posMS <= marker2.posMS) {
            mLoopBack = mStreamMs <= marker2.posMS && marker2.posMS < f9;
        }
        if (mLoopBack) {
            float f10;
            if (jp) {
                f10 = MsToTick(marker2.posMS) - 1.0f;
            } else {
                f10 = mSongPos.GetTotalTick();
            }
            if (mMidiParserMgr) {
                mMidiParserMgr->Reset(f10);
            }
            mLoopBackFromStreamMs = mStreamMs;
            mLastLoopFromMs = marker1.posMS;
            mLastLoopToMs = marker2.posMS;
            static Message msg("stream_jump");
            Export(msg, true);
        }
        mStreamMs = f9;
        if (mMidiParserMgr) {
            mMidiParserMgr->Poll();
        }
        CheckBeat();
        CheckLevels();
        mAudio->Poll();
    }
}

void HamMaster::Jump(float ms) {
    mSongPos = mSongData->CalcSongPos(this, ms);
    const SongPos &tmp = mSongPos;
    mLastSongPos = tmp;
    mCurActivePlayers = -1;
    mActivePlayerGemIdx = 0;
    if (mMidiParserMgr) {
        mMidiParserMgr->Reset(tmp.GetTotalTick());
    }
    mAudio->Jump(ms);
}

void HamMaster::Reset() {
    mLastSongPos = SongPos();
    mActivePlayerGemIdx = 0;
    mCurActivePlayers = -1;
    for (int i = 0; i < mSubmixIdxs.size(); i++) {
        mSubmixIdxs[i] = 0;
    }
    if (mMidiParserMgr) {
        mMidiParserMgr->Reset();
    }
    static Message msg("first_beat");
    Export(msg, true);
    ResetAudio();
    mStreamMs = 0;
    mLoopBackFromStreamMs = 0;
    mLoopBack = false;
    mLastStreamMs = 0;
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
    SongInfo *song_info,
    bool stream,
    int num_difficulties,
    bool synchronous,
    HamSongDataValidate validateMode,
    std::vector<MidiReceiver *> *receivers
) {
    mSongStream = stream;
    mSongInfo = song_info;
    mSongData->Load(song_info, synchronous, validateMode);
    MILO_ASSERT(!mLoader, 0x69);
    mLoader = new HamMasterLoader(this);
    mLoaded = false;
    if (synchronous) {
        TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
    }
}

void HamMaster::LoadOnlySongData(
    SongInfo *song_info, bool synchronous, HamSongDataValidate validateMode
) {
    mSongInfo = song_info;
    mSongData->Load(song_info, synchronous, validateMode);
}

void HamMaster::ResetAudio() {
    if (mAudio && mAudio->GetTime()) {
        mAudio->Jump(0);
    }
}

float HamMaster::StreamMs() const { return mStreamMs; }

bool HamMaster::DetectStreamJump(float &f1, float &f2, float &f3) const {
    if (!mLoopBack) {
        return false;
    } else {
        f1 = mLoopBackFromStreamMs;
        f2 = mLastLoopFromMs;
        f3 = mLastLoopToMs;
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
        mAudio->Load(mSongInfo, mSongStream);
        mSongInfo = nullptr;
        if (mMidiParserMgr) {
            mMidiParserMgr->FinishLoad();
        }
        mLoaded = true;
        RELEASE(mLoader);
    }
}

void HamMaster::CheckBeat() {
    int totalbeat1 = mLastSongPos.GetTotalBeat();
    int totalbeat2 = mSongPos.GetTotalBeat();
    if (totalbeat1 != totalbeat2) {
        int beat = mSongPos.GetBeat();
        TheHamProvider->SetProperty("beat", beat + 1);
        if (beat == 0) {
            if (mMetronome) {
                TheSynth->PlaySound("metronome_measure", 0, 0, 0);
            }
        } else {
            if (mMetronome) {
                TheSynth->PlaySound("metronome_beat", 0, 0, 0);
            }
        }
        static DataNode &n = DataVariable("beat");
        n = totalbeat2;
        static Message msg("beat");
        Export(msg, true);
    }
    if (mLastSongPos.GetMeasure() != mSongPos.GetMeasure()) {
        static DataNode &n = DataVariable("measure");
        n = mSongPos.GetMeasure();
        static Message msg("downbeat");
        TheHamProvider->Export(msg, true);
    }
    if (mLastSongPos.GetTick() / 240 != mSongPos.GetTick() / 240) {
        static Message msg("halfbeat");
        TheHamProvider->Export(msg, true);
    }
    if (mLastSongPos.GetTick() / 120 != mSongPos.GetTick() / 120) {
        static Message msg("quarterbeat");
        TheHamProvider->Export(msg, true);
    }
    mLastSongPos = mSongPos;
}

void HamMaster::CheckLevels() {
    static float sFloat1c0 = 40;
    if (TheSynth) {
        PropertyEventProvider *prov =
            ObjectDir::Main()->Find<PropertyEventProvider>("audio_channels", false);
        if (prov) {
            const auto &levelData = TheSynth->GetLevelData();
            if (levelData.size() != 0) {
                float f12 = levelData[levelData.size() - 1].mRMS;
                float f13;
                if (levelData.size() > 2) {
                    f13 = levelData[levelData.size() - 2].mRMS;
                } else {
                    f13 = f12;
                }
                f12 = (RatioToDb(f12) + sFloat1c0) / sFloat1c0;
                f13 = (RatioToDb(f13) + sFloat1c0) / sFloat1c0;

                ClampEq(f12, 0.0f, 1.0f);
                ClampEq(f13, 0.0f, 1.0f);

                mLevelHistorisis.push_back(Vector2(f12, f13));

                float f14 = 0;
                float f15 = 0;
                FOREACH (it, mLevelHistorisis) {
                    Vector2 &cur = *it;
                    f15 += cur.x;
                    f14 += cur.y;
                }
                unsigned int numVecs = mLevelHistorisis.size();
                float f15div = (1 / (float)numVecs) * f15;
                float f14div = (1 / (float)numVecs) * f14;

                while (mLevelHistorisis.size() > 3) {
                    mLevelHistorisis.pop_front();
                }

                float channels[8];
                float firstloc = 1;
                for (int i = 0; i < 4; i++) {
                    channels[3 - i] = firstloc * f15div;
                    firstloc *= 0.7f;
                }
                float secondloc = 1;
                for (int i = 4; i < 8; i++) {
                    channels[i] = secondloc * f14div;
                    secondloc *= 0.7f;
                }
                for (int i = 0; i < 8; i++) {
                    prov->SetProperty(MakeString("channel%d", i), channels[i]);
                }
            }
        }
    }
}

HamMasterLoader::HamMasterLoader(HamMaster *master)
    : Loader("", kLoadBack), mMaster(master) {}

void HamMasterLoader::PollLoading() { mMaster->LoaderPoll(); }
