#include "utl/Song.h"
#include "beatmatch/HxAudio.h"
#include "midi/MidiParser.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Overlay.h"
#include "rndobj/Poll.h"
#include "synth/Synth.h"
#include "utl/BeatMap.h"
#include "utl/FakeSongMgr.h"

SongCallback *Song::sCallback;
bool Song::mFastSync;

#pragma region Hmx::Object

Song::Song()
    : mHxMaster(nullptr), mHxSongData(nullptr), mDebugParsers(this), mSongEndFrame(0),
      mSpeed(1), mLoopPoints(0, 0), mDirty(true) {
    SetName("Song", ObjectDir::Main());
    static DataNode &n = DataVariable("tool_song");
    n = this;
}

Song::~Song() {
    static DataNode &n = DataVariable("tool_song");
    n = NULL_OBJ;
    Unload();
}

BEGIN_HANDLERS(Song)
    HANDLE_EXPR(get_bookmarkers, GetBookmarks())
    HANDLE_EXPR(get_midi_parsers, GetMidiParsers())
    HANDLE_ACTION(
        jump_to,
        _msg->Type(2) == kDataSymbol ? JumpTo(_msg->Sym(2)) : JumpTo(_msg->Int(2))
    )
    HANDLE_ACTION(sync_state, SyncState())
    HANDLE_ACTION(set_loop_start, SetLoopStart(_msg->Float(2)))
    HANDLE_ACTION(set_loop_end, SetLoopEnd(_msg->Float(2)))
    HANDLE_ACTION(play, Play())
    HANDLE_ACTION(pause, Pause())
    HANDLE_EXPR(song_name, mSongName)
    HANDLE(mbt_from_seconds, OnMBTFromSeconds)
    HANDLE_EXPR(
        seconds_from_mbt, GetFrameFromMBT(_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE(mbt_from_tick, OnMBTFromTick)
    HANDLE_EXPR(tick_from_mbt, GetTickFromMBT(_msg->Int(2), _msg->Int(3), _msg->Int(4)))
    HANDLE_ACTION(add_section, AddSection(_msg->Sym(2), _msg->Float(3)))
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(MBT)
    SYNC_PROP(m, o.measure)
    SYNC_PROP(b, o.beat)
    SYNC_PROP(t, o.tick)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(Song)
    SYNC_PROP_SET(song, mSongName, SetSong(_val.Sym()))
    SYNC_PROP_MODIFY(speed, mSpeed, SetSpeed())
    SYNC_PROP_MODIFY(debug_parsers, mDebugParsers, UpdateDebugParsers())
    SYNC_PROP(loop_start, mLoopStart)
    SYNC_PROP(loop_end, mLoopEnd)
    SYNC_PROP(song_end_frame, mSongEndFrame)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Song)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mSongName;
    bs << mDirty;
END_SAVES

INIT_REVS(0, 0)

BEGIN_LOADS(Song)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    RndAnimatable::Load(bs);
    static Symbol sSongName;
    bs >> sSongName;
    if (mSongName != sSongName) {
        SetSong(sSongName);
    }
    static bool sDirty;
    d >> sDirty;
    if (sDirty) {
        mDirty = true;
    }
END_LOADS

#pragma endregion
#pragma region RndAnimatable

void Song::SetFrame(float frame, float blend) {
    bool paused = false;
    if (mHxMaster) {
        paused = mHxMaster->GetHxAudio()->Paused();
    }
    if (paused && mLoopPoints.y < frame || frame < mLoopPoints.x) {
        if (frame <= mLoopPoints.y) {
            frame = mLoopPoints.x;
        } else {
            frame = frame - (mLoopPoints.y - mLoopPoints.x);
        }
        SetStateDirty(true);
    }
    frame = Min(frame, StartFrame(), EndFrame());
    RndAnimatable::SetFrame(frame, blend);
    if (paused) {
        if (mHxMaster) {
            mHxMaster->Poll(frame * 1000.0f);
        }
        if (mDirty) {
            SyncState();
        }
    } else if (GetFrame() != frame) {
        SetStateDirty(true);
    }
}

#pragma endregion
#pragma region MidiReceiver

void Song::OnText(int tick, const char *text, unsigned char type) {
    static bool sListening;
    if (type == 3) {
        sListening = streq(text, "EVENTS");
    }
    if (sListening) {
        if (strneq(text, "[section ", 9)) {
            String str(text);
            str = str.substr(9, str.length() - 10);
            AddSection(str.c_str(), GetBeatMap()->Beat(tick));
        }
    }
}

#pragma endregion
#pragma region RndOverlay::Callback

float Song::UpdateOverlay(RndOverlay *overlay, float f2) {
    FOREACH (it, mDebugParsers) {
        DataNode handled = (*it)->Handle(Message("debug_draw", f2, GetBeat()), true);
        f2 += handled.Float();
    }
    return f2;
}

#pragma endregion
#pragma region Song

DataNode Song::OnMBTFromSeconds(const DataArray *da) {
    float frame = da->Float(2);
    MBT mbt = GetMBTFromFrame(frame, nullptr);
    *da->Var(3) = mbt.measure;
    *da->Var(4) = mbt.beat;
    *da->Var(5) = mbt.tick;
    return 0;
}

DataNode Song::OnMBTFromTick(const DataArray *da) {
    int tick = da->Int(2);
    MBT mbt = GetMBTFromTick(tick, nullptr);
    *da->Var(3) = mbt.measure;
    *da->Var(4) = mbt.beat;
    *da->Var(5) = mbt.tick;
    return 0;
}

void Song::Unload() {
    RELEASE(mHxMaster);
    RELEASE(mHxSongData);
    mSongSections.clear();
}

TempoMap *Song::GetTempoMap() {
    if (mHxSongData)
        return mHxSongData->GetTempoMap();
    else
        return nullptr;
}

BeatMap *Song::GetBeatMap() {
    if (mHxSongData)
        return mHxSongData->GetBeatMap();
    else
        return nullptr;
}

MeasureMap *Song::GetMeasureMap() {
    if (mHxSongData)
        return mHxSongData->GetMeasureMap();
    else
        return nullptr;
}

void Song::SetSong(Symbol song) {
    mSongName = song;
    Load();
}

MBT Song::GetMBTFromFrame(float frame, int *bpm) {
    MBT ret;
    int tick = 0;
    if (GetTempoMap()) {
        tick = GetTempoMap()->TimeToTick(frame * 1000.0f + 0.5f);
    }
    ret = GetMBTFromTick(tick, bpm);
    return ret;
}

MBT Song::GetMBTFromTick(int tick, int *bpm) {
    MBT ret;
    int oBPM = 0;
    if (GetMeasureMap()) {
        GetMeasureMap()->TickToMeasureBeatTick(
            tick, ret.measure, ret.beat, ret.tick, oBPM
        );
    }
    ret.measure++;
    ret.beat++;
    if (bpm)
        *bpm = oBPM;
    return ret;
}

DataNode Song::GetMidiParsers() {
    DataArrayPtr ptr(new DataArray(0));
    if (TheMidiParserMgr) {
        FOREACH (it, MidiParser::Parsers()) {
            String str((*it)->Name());
            if (str != "events_parser") {
                ptr->Insert(ptr->Size(), *it);
            }
        }
    }
    return ptr;
}

void Song::Play() {
    if (mHxMaster) {
        sCallback->SongPlay(true);
        mHxMaster->Jump(GetFrame() * 1000.0f);
        while (!mHxMaster->GetHxAudio()->IsReady()) {
            TheSynth->Poll();
            mHxMaster->GetHxAudio()->Poll();
        }
        mHxMaster->GetHxAudio()->SetPaused(false);
    }
}

void Song::Pause() {
    if (mHxMaster) {
        sCallback->SongPlay(false);
        if (mHxMaster) {
            mHxMaster->GetHxAudio()->SetPaused(true);
        }
    }
}

void Song::SetSpeed() {
    if (mHxMaster) {
        mHxMaster->GetHxAudio()->GetSongStream()->SetSpeed(mSpeed);
    }
}

float Song::GetBeat() {
    if (!GetTempoMap()) {
        return 0;
    } else {
        float tick = GetTempoMap()->TimeToTick(GetFrame() * 1000.0f);
        return GetBeatMap()->Beat(tick);
    }
}

int Song::GetTickFromMBT(int m, int b, int t) {
    if (GetMeasureMap()) {
        return GetMeasureMap()->MeasureBeatTickToTick(m - 1, b - 1, t);
    } else {
        return 0;
    }
}

void Song::Load() {
    std::vector<Symbol> vec;
    FOREACH (it, mDebugParsers) {
        vec.push_back((*it)->Name());
    }
    MILO_ASSERT(sCallback, 0xF8);
    sCallback->Preload();
    Unload();
    if (mSongName.Null())
        return;
    else
        LoadSong();
}

void Song::JumpTo(Symbol section) {
    int jumpTick = 0;
    FOREACH (it, mSongSections) {
        if (section == it->second) {
            jumpTick = it->first;
            break;
        }
    }
    JumpTo(jumpTick);
}

void Song::JumpTo(int tick) {
    float f = 0;
    if (mHxSongData) {
        f = mHxSongData->GetTempoMap()->TickToTime(tick) / 1000.0f;
    }
    MILO_ASSERT(sCallback, 0x1AB);
    sCallback->SongSetFrame(this, f);
    SyncState();
}

void Song::LoadSong() {
    DataArray *cfg = TheFakeSongMgr->GetSongConfig(mSongName);
    CreateSong(mSongName, cfg, &mHxSongData, &mHxMaster);
    if (mHxSongData && mHxMaster) {
        RndPollable *poll = dynamic_cast<RndPollable *>(MainDir());
        if (poll)
            poll->Enter();
        mSongEndFrame = mHxMaster->SongDurationMs() / 1000.0f;
        if (mSongName != unk54) {
            SetLoopStart(0);
            SetLoopEnd(mSongEndFrame);
        } else {
            if (mLoopPoints.x > mSongEndFrame)
                SetLoopStart(mSongEndFrame);
            if (mLoopPoints.y > mSongEndFrame)
                SetLoopEnd(mSongEndFrame);
        }
        JumpTo(0);
        sCallback->ProcessBookmarks(GetBookmarks());
        if (SystemConfig("milo_tool")->FindInt("mute_song")) {
            mHxMaster->GetHxAudio()->SetMasterVolume(-96.0f);
        }
        unk54 = mSongName;
    } else {
        MILO_NOTIFY("Could not create song");
    }
}

void Song::AddSection(Symbol section, float beat) {
    int tick = GetBeatMap()->BeatToTick(beat);
    mSongSections[tick] = section;
}

void Song::UpdateDebugParsers() {
    RndOverlay *o = RndOverlay::Find("song", true);
    if (o) {
        o->SetCallback(this);
        o->SetShowing(mDebugParsers.size() > 0);
    }
}

void Song::SetLoopStart(float f) {
    mLoopPoints.x = f;
    mLoopStart = GetMBTFromFrame(f, nullptr);
    if (mLoopPoints.y < f)
        SetLoopEnd(f);
}

void Song::SetLoopEnd(float f) {
    mLoopPoints.y = f;
    mLoopEnd = GetMBTFromFrame(f, nullptr);
    if (mLoopPoints.x > f)
        SetLoopStart(f);
}

void Song::SetStateDirty(bool dirty) {
    mDirty = dirty;
    DataNode name = Symbol(Name());
    sCallback->UpdateObject(this, DataArrayPtr(name));
}

DataNode Song::GetBookmarks() {
    DataArrayPtr ptr(new DataArray(mSongSections.size() + 1));
    ptr->Node(0) = "song_begin";
    int idx = 1;
    FOREACH (it, mSongSections) {
        ptr->Node(idx) = it->second;
        idx++;
    }
    return ptr;
}

ObjectDir *Song::MainDir() const {
    MILO_ASSERT(sCallback, 0x2A9);
    return sCallback->SongMainDir();
}

float Song::GetFrameFromMBT(int m, int b, int t) {
    int tick = GetTickFromMBT(m, b, t);
    if (GetTempoMap()) {
        return GetTempoMap()->TickToTime(tick) / 1000.0f;
    } else {
        return 0;
    }
}
