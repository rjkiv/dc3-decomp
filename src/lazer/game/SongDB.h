#pragma once

#include "beatmatch/HxMaster.h"
#include "hamobj/HamSongData.h"
#include "midi/DataEventList.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/SongPos.h"

class SongDB {
public:
    SongDB();
    ~SongDB();
    SongPos CalcSongPos(HxMaster *, float);
    void PostLoad(DataEventList *);

    MEM_OVERLOAD(SongDB, 0x1c);

    HamSongData *unk0;
    float unk4;

private:
    void ParseEvents(DataEventList *);
};

extern SongDB *TheSongDB;
