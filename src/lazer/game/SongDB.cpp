#include "lazer/game/SongDB.h"
#include "SongDB.h"
#include "beatmatch/HxMaster.h"
#include "hamobj/HamSongData.h"
#include "macros.h"
#include "midi/DataEventList.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/SongPos.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"

SongDB *TheSongDB;

SongDB::SongDB() : mSongData(new HamSongData()), mSongDurationMs(0) {}
SongDB::~SongDB() { RELEASE(mSongData); }
SongPos SongDB::CalcSongPos(HxMaster *hx, float f) {
    return mSongData->CalcSongPos(hx, f);
}
void SongDB::PostLoad(DataEventList *list) { ParseEvents(list); }
float SongDB::GetSongDurationMs() const { return mSongDurationMs; }

void SongDB::ParseEvents(DataEventList *list) {
    mSongDurationMs = 0.0f;
    for (int i = 0; i < list->Size(); i++) {
        static Symbol coda("coda");
        static Symbol end("end");
        const DataEvent &event = list->Event(i);
        Symbol sym = event.Msg()->Sym(1);
        if (sym == end) {
            if (mSongDurationMs != 0.0f) {
                MILO_FAIL("Duplicate end text event");
            }
            mSongDurationMs = BeatToMs(event.start);
            return;
        }
    }
}
