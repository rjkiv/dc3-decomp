#include "hamobj/HamSongData.h"
#include "macros.h"
#include "math/FileChecksum.h"
#include "math/StreamChecksum.h"
#include "midi/Midi.h"
#include "midi/MidiParserMgr.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/BeatMap.h"
#include "utl/MemStream.h"
#include "utl/SongPos.h"
#include "utl/TempoMap.h"

HamSongData::HamSongData()
    : unk10(0), unk14(0), mTempoMap(nullptr), mMeasureMap(nullptr), mBeatMap(nullptr),
      mStream(0), mMidiReader(0) {
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

// SongPos HamSongData::CalcSongPos(HxMaster* master, float f1){
//     MILO_ASSERT(mTempoMap, 0xB9);
//     MILO_ASSERT(mMeasureMap, 0xBA);
//     MILO_ASSERT(mBeatMap, 0xBB);
// }

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
    unk10 = true;
}

void Validate(MemStream *ms, const char *cc, bool b) {
    if (UsingCD()) {
        if (HasFileChecksumData()) {
            StreamChecksumValidator v;
            if (v.Begin(cc, b)) {
                v.Update((const unsigned char *)ms->Buffer(), ms->BufferSize());
                v.End();
                v.Validate();
            }
        }
    }
}

void HamSongData::Load(const char *cc, const SongInfo *info, bool b3) {
    unk14 = info;
    ResetTheTempoMap();
    ResetTheBeatMap();
    RELEASE(mTempoMap);
    RELEASE(mMeasureMap);
    RELEASE(mBeatMap);
    mBeatMap = new BeatMap();
    MidiReceiver *rcvr = TheMidiParserMgr ? TheMidiParserMgr : (MidiReceiver *)this;
    mMidiReader = new MidiReader(*mStream, *rcvr, cc);
    if (b3) {
        while (!Poll())
            ;
    }
}
