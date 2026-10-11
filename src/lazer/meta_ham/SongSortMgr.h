#pragma once
#include "NavListSortMgr.h"
#include "SongRecord.h"
#include "SongSort.h"
#include "utl/Symbol.h"

class SongSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool HeadersSelectable(); // 0x6c
    virtual bool SelectionIs(Symbol);
    virtual bool DataIs(int, Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();
    virtual int GetListIndexFromHeaderIndex(int);

    void MarkElementsProvided(UIListProvider *);
    void MarkElementInPlaylist(Symbol, bool);
    void OnHighlightChanged();
    void OnSetlistModeChanged();
    void OnSetlistChanged();
    void SetQuasiRandomSong();
    void RebuildSongRecordMap();
    void SetupQuasiRandomSongs();
    void SetSetlistMode(bool);
    bool GetSetlistMode();
    Symbol DetermineHeaderSymbolForSong(Symbol);
    int FirstArtistSongIndex(Symbol);

    static void Init(SongPreview &);
    static void Terminate();

    std::map<Symbol, SongRecord> &GetSongRecords() { return mSongs; }
    unsigned int MapSize() { return mSongs.size(); }

private:
    SongSortMgr(SongPreview &);
    virtual ~SongSortMgr();

    std::map<Symbol, SongRecord> mSongs; // 0x78
    int mRankedSongs; // 0x90 - not in pdb
    std::vector<int> mQuasiRandomIndices; // 0x94
};

extern SongSortMgr *TheSongSortMgr;
