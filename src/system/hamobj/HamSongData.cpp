#include "hamobj/HamSongData.h"
#include "hamobj/HamMaster.h"
#include "macros.h"
#include "math/FileChecksum.h"
#include "math/StreamChecksum.h"
#include "midi/Midi.h"
#include "midi/MidiParserMgr.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/BeatMap.h"
#include "utl/FakeSongMgr.h"
#include "utl/FileStream.h"
#include "utl/MemMgr.h"
#include "utl/MemStream.h"
#include "utl/SongInfoCopy.h"
#include "utl/SongPos.h"
#include "utl/TempoMap.h"

HamSongData::HamSongData()
    : mLoaded(0), mSongInfo(0), mTempoMap(nullptr), mMeasureMap(nullptr),
      mBeatMap(nullptr), mStream(0), mMidiReader(0) {
    sInstance = this;
}

HamSongData::~HamSongData() {
    if (sInstance == this) {
        sInstance = nullptr;
    }
    ResetTheTempoMap();
    ResetTheBeatMap();
    RELEASE(mTempoMap);
    RELEASE(mMeasureMap);
    RELEASE(mBeatMap);
}

SongPos HamSongData::CalcSongPos(HxMaster *master, float ms) {
    MILO_ASSERT(mTempoMap, 0xB9);
    MILO_ASSERT(mMeasureMap, 0xBA);
    MILO_ASSERT(mBeatMap, 0xBB);
    HamMaster *hamMaster = dynamic_cast<HamMaster *>(master);
    if (hamMaster) {
        Stream *stream = hamMaster->GetAudio()->GetSongStream();
        if (stream) {
            ms += stream->GetJumpBackTotalTime(ms);
        }
    }
    float tick = mTempoMap->TimeToTick(ms);
    int m, b, t, bpm;
    mMeasureMap->TickToMeasureBeatTick(tick, m, b, t, bpm);
    float beat = mBeatMap->Beat((int)tick);
    return SongPos(tick, beat, 0, m, b, t);
}

void HamSongData::SetMaps() {
    SetTheTempoMap(mTempoMap);
    SetTheBeatMap(mBeatMap);
}

bool HamSongData::Poll() {
    if (mMidiReader) {
        bool ret = mMidiReader->ReadSomeEvents(20);
        if (ret) {
            PostLoad();
        }
        return ret;
    } else
        return true;
}

void HamSongData::PostLoad() {
    mMidiReader->ClaimMaps(mMeasureMap, mTempoMap);
    SetMaps();
    RELEASE(mMidiReader);
    RELEASE(mStream);
    MILO_ASSERT(mTempoMap, 0xA5);
    mTempoMap->Finalize();
    mLoaded = true;
}

void Validate(MemStream *ms, const char *cc, bool b) {
    if (UsingCD()) {
        if (HasFileChecksumData()) {
            StreamChecksumValidator v;
            if (v.Begin(cc, b)) {
                v.Update((const unsigned char *)ms->Buffer(), ms->Size());
                v.End();
                v.Validate();
            }
        }
    }
}

void HamSongData::Load(const char *name, const SongInfo *song_info, bool synchronous) {
    mSongInfo = song_info;
    ResetTheTempoMap();
    ResetTheBeatMap();
    RELEASE(mTempoMap);
    RELEASE(mMeasureMap);
    RELEASE(mBeatMap);
    mBeatMap = new BeatMap();
    MidiParserMgr *mgr = TheMidiParserMgr;
    MidiReceiver *rcvr = mgr ? mgr : (MidiReceiver *)this;
    mMidiReader = new MidiReader(*mStream, *rcvr, name);
    if (synchronous) {
        while (!Poll())
            ;
    }
}

void HamSongData::Load(
    const SongInfo *song_info, bool synchronous, HamSongDataValidate validateMode
) {
    const char *midi = FakeSongMgr::MidiFile(song_info);
    FileStream fStream(midi, FileStream::kRead, false);
    mStream = new MemStream();
    {
        MemDoTempAllocations tmp;
        mStream->Resize(fStream.Size());
        fStream.Read((void *)mStream->Buffer(), fStream.Size());
        if (validateMode == 2 || validateMode == 1) {
            Validate(mStream, midi, validateMode == 1);
        }
    }
    Load(midi, song_info, synchronous);
}
