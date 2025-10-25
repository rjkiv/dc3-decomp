#pragma once

#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "utl/Symbol.h"
class Playlist {
public:
    virtual ~Playlist();

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
    virtual void HandleChange();
};

int GetDynamicPlaylistID(Symbol);
