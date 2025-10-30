#pragma once

#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "utl/Symbol.h"

enum PlaylistType { // Taken from RB3 and "Setlist" replaced with "Playlist"
    kPlaylistLocal = 0,
    kPlaylistInternal = 1,
    kPlaylistFriend = 2,
    kPlaylistHarmonix = 3,
    kBattleHarmonix = 4,
    kBattleFriend = 5,
    kBattleHarmonixArchived = 6,
    kBattleFriendArchived = 7
};

class Playlist {
public:
    virtual ~Playlist(); // 0x0
    // unsure where these go but they are definitely between 0x4 - 0xc
    virtual bool IsDirty() { return 0; }
    virtual bool IsCustom() const { return 0; }
    virtual void SetOnlineID(int) {}
    //
    virtual int GetOnlineID() { return -1; } // 0x10 ?
    virtual PlaylistType GetType() const; // 0x14

    Playlist();
    void SwapSongs(int, int);
    void MoveSong(int, int);
    void ShuffleSongs();
    bool IsValidSong(int) const;
    int GetSong(int) const;
    int GetDuration() const;
    int GetSongDuration(int) const;
    void RemoveSong();
    int GetLastValidSongIndex() const;
    void RemoveSongAtIndex(int);
    void AddSong(int);
    void Clear();
    void InsertSong(int, int);
    int GetNumSongs() const;

    Symbol unk4;
    bool unk8;
    bool unk9;
    std::vector<int> m_vSongs; // 0xc

protected:
    virtual void HandleChange() {}
};

class CustomPlaylist : public Playlist, public FixedSizeSaveable {
public:
    virtual ~CustomPlaylist();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    CustomPlaylist();
    void SetParentProfile(class HamProfile *);
    static int SaveSize(int);
    void Copy(CustomPlaylist *);

    HamProfile *unk20;
    bool unk24;
    int unk28;

protected:
    virtual void HandleChange(); // 0x18
};

int GetDynamicPlaylistID(Symbol);
