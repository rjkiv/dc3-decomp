#pragma once
#include "net_ham/RCJobDingo.h"
#include "types.h"
#include "utl/Str.h"

class SongQueueRow {
public:
    int songQueueID; // 0x0
    int songID; // 0x4
    String artist; // 0x8
    String song; // 0x10
};

class SetPartyOptionsJob : public RCJob {
public:
    SetPartyOptionsJob(Hmx::Object *callback, const char *onlineID);
};

class GetPartyOptionsJob : public RCJob {
public:
    GetPartyOptionsJob(Hmx::Object *callback, const char *onlineID);
    void GetOptions();
};

class GetPartySongQueueJob : public RCJob {
public:
    GetPartySongQueueJob(Hmx::Object *callback, const char *onlineID);
    void GetSongQueue(std::list<SongQueueRow> *);
};

class AddSongToPartySongQueueJob : public RCJob {
public:
    AddSongToPartySongQueueJob(Hmx::Object *callback, const char *onlineID, int songID);
};

class SyncPlayerSongsJob : public RCJob {
public:
    SyncPlayerSongsJob(Hmx::Object *callback, const char *onlineID, String &songIDs);
};

class DeleteSongFromPartySongQueueJob : public RCJob {
public:
    DeleteSongFromPartySongQueueJob(
        Hmx::Object *callback, const char *onlineID, int songID
    );
};
