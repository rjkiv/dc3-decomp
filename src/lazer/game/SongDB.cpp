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

SongDB::SongDB() {
    unk0 = new HamSongData();
    unk4 = 0.0f;
}

SongDB::~SongDB() { RELEASE(unk0); }

SongPos SongDB::CalcSongPos(HxMaster *hx, float f) { return SongPos(); }

void SongDB::PostLoad(DataEventList *list) { ParseEvents(list); }

void SongDB::ParseEvents(DataEventList *list) {
    unk4 = 0.0f;
    for (int i = 0; i < list->Size(); i++) {
        static Symbol coda("coda");
        static Symbol end("end");
        const DataEvent &event = list->Event(i);
        if (event.Msg()->Sym(1) == end) {
            if (unk4 != 0.0f) {
                MILO_FAIL("Duplicate end text event");
            }
            unk4 = BeatToMs(event.Start());
            return;
        }
    }
}
