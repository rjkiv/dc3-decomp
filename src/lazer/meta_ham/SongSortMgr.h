#pragma once
#include "NavListSortMgr.h"
#include "SongRecord.h"
#include "SongSort.h"

class SongSortMgr : public NavListSortMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SelectionIs(Symbol);
    virtual bool HeadersSelectable();
    virtual int GetListIndexFromHeaderIndex(int);
    virtual bool DataIs(int, Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();

    void MarkElementsProvided(UIListProvider *);
    void MarkElementInPlaylist(Symbol, bool);
    void OnHighlightChanged();
    void OnSetlistModeChanged();
    void OnSetlistChanged();
    void SetQuasiRandomSong();
    void RebuildSongRecordMap();
    void SetupQuasiRandomSongs();
    void SetSetlistMode(bool);
    Symbol DetermineHeaderSymbolForSong(Symbol);
    int FirstArtistSongIndex(Symbol);

    static void Init(SongPreview &);
    static void Terminate();

private:
    SongSortMgr(SongPreview &);
    virtual ~SongSortMgr();

protected:
    std::map<Symbol, SongRecord> unk78; // 0x78
    double *unk7c; // 0x7c
    double *unk80; // 0x80
    double *unk84; // 0x84
    double *unk88; // 0x88
    bool unk8c; // 0x8c
    int unk90; // 0x90
    std::vector<int> unk94; // 0x94
    // double *unk98; // 0x98
    // double *unk9c; // 0x9c
};

extern SongSortMgr *TheSongSortMgr;
