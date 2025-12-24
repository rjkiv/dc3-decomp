#pragma once
#include "net_ham/RCJobDingo.h"
#include "types.h"
#include "utl/Str.h"

class SongQueueRow {
public:
    SongQueueRow();
    SongQueueRow(SongQueueRow const &);

    const SongQueueRow *unk0;
    u32 unk4;
    String unk8;
    String unk10;
};

class SetPartyOptionsJob {};

class GetPartyOptionsJob : public RCJob {
public:
    GetPartyOptionsJob(Hmx::Object *, char const *);
};

class GetPartySongQueueJob : public RCJob {
public:
    GetPartySongQueueJob(Hmx::Object *, char const *);
};

class AddSongToPartySongQueueJob : public RCJob {
public:
    AddSongToPartySongQueueJob(Hmx::Object *, char const *, int);
};

class SyncPlayerSongsJob : public RCJob {
public:
    SyncPlayerSongsJob(Hmx::Object *, char const *, String &);
};

class DeleteSongFromPartySongQueueJob : RCJob {
public:
    DeleteSongFromPartySongQueueJob(Hmx::Object *, char const *, int);
};
