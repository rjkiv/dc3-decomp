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
    Playlist();
    virtual ~Playlist(); // 0x0
    virtual bool IsCustom() const { return false; } // 0x4
    virtual void SetOnlineID(int) {} // 0x8
    virtual int GetOnlineID() { return -1; } // 0xc
    virtual bool IsDirty() { return false; } // 0x10
    virtual PlaylistType GetType() const {
        if (unk9) {
            return (PlaylistType)2;
        } else if (unk8) {
            return (PlaylistType)4;
        } else {
            return IsCustom() ? (PlaylistType)1 : (PlaylistType)3;
        }
    } // 0x14

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
    bool IsEmpty() const { return m_vSongs.empty(); }
    bool IsFull() const { return m_vSongs.size() >= 20; }
    Symbol GetName() const { return mName; }
    void SetName(Symbol name) { mName = name; }
    void SetUnk8(bool b) { unk8 = b; }
    bool GetUnk8() const { return unk8; }

protected:
    virtual void HandleChange() {}

    Symbol mName; // 0x4
    bool unk8; // 0x8
    bool unk9; // 0x9
    std::vector<int> m_vSongs; // 0xc
};

class CustomPlaylist : public Playlist, public FixedSizeSaveable {
public:
    CustomPlaylist();
    virtual ~CustomPlaylist();
    virtual bool IsCustom() const { return true; } // 0x4
    virtual void SetOnlineID(int id) { mOnlineID = id; } // 0x8
    virtual int GetOnlineID() { return mOnlineID; } // 0xc
    virtual bool IsDirty() { return false; } // 0x10
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void SetParentProfile(class HamProfile *);
    void Copy(CustomPlaylist *);

    static int SaveSize(int);

    HamProfile *mProfile; // 0x20
    bool unk24; // 0x24
    int mOnlineID; // 0x28

protected:
    virtual void HandleChange(); // 0x18
};

int GetDynamicPlaylistID(Symbol);
