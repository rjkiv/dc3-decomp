#pragma once
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "utl/Symbol.h"

enum PlaylistType { // Taken from RB3 and "Setlist" replaced with "Playlist"
    ENDLESS = 0x0000,
    CUSTOM = 0x0001,
    ERA = 0x0002,
    CREW = 0x0003,
    FITNESS = 0x0004
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
        if (m_bIsEra) {
            return ERA;
        } else if (m_bIsFitness) {
            return FITNESS;
        } else {
            return IsCustom() ? CUSTOM : CREW;
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
    Symbol GetName() const { return m_symName; }
    void SetName(Symbol name) { m_symName = name; }
    void SetIsFitness(bool b) { m_bIsFitness = b; }
    bool IsFitness() const { return m_bIsFitness; }
    void SetIsEra(bool b) { m_bIsEra = b; }
    bool IsEra() const { return m_bIsEra; }

protected:
    virtual void HandleChange() {}

    Symbol m_symName; // 0x4
    bool m_bIsFitness; // 0x8
    bool m_bIsEra; // 0x9
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

protected:
    virtual void HandleChange(); // 0x18

private:
    HamProfile *m_pParentProfile; // 0x20
    bool mDirty; // 0x24
    int mOnlineID; // 0x28
};

int GetDynamicPlaylistID(Symbol);
