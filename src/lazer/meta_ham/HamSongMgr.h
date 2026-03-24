#pragma once
#include "HamSongMetadata.h"
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Playlist.h"
#include "meta/DataArraySongInfo.h"
#include "meta/Jukebox.h"
#include "meta/SongMetadata.h"
#include "meta/SongMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "stl/_map.h"
#include "stl/_vector.h"
#include "utl/BinStream.h"
#include "utl/SongInfoCopy.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

enum SongType {
};

class HamSongMgr : public SongMgr {
public:
    HamSongMgr();
    virtual DataNode Handle(DataArray *, bool);
    // ContentMgr::Callback
    virtual void ContentMounted(char const *contentName, char const *c2) {
        SongMgr::ContentMounted(contentName, c2);
    }
    virtual void ContentDone();
    virtual char const *ContentPattern() { return "songs*.dta"; }
    virtual char const *ContentDir() { return "songs"; }
    virtual bool HasContentAltDirs() { return mContentAltDirs.size() > 0; }
    // SongMgr
    virtual void Init();
    virtual void Terminate();
    virtual const HamSongMetadata *Data(int songID) const;
    virtual SongInfo *SongAudioData(int songID) const;
    virtual Symbol GetShortNameFromSongID(int songID, bool fail = true) const;
    virtual int GetSongIDFromShortName(Symbol shortname, bool fail = true) const;

    const char *GetAlbumArtPath(Symbol) const;
    bool IsDummySong(Symbol) const;
    void AddSongs(DataArray *);
    void AddRecentSong(Symbol);
    Symbol GetArtistNameFromShortName(Symbol);
    Playlist *GetPlaylist(Symbol);
    Playlist *GetPlaylistWithLocalizedName(String);
    Playlist *GetPlaylist(int);
    char const *BarksFile(Symbol) const;
    int GetDuration(Symbol) const;
    Symbol GetCharacter(Symbol) const;
    int GetBPM(Symbol) const;
    Symbol RankTierToken(int) const;
    int RankTier(int rank) const;
    int RankTier(Symbol shortname) const;
    void GetCoreStarsForDifficulty(HamProfile const *, Difficulty, int &, int &) const;
    void GetCharacterStars(HamProfile const *, Symbol, int &, int &) const;
    void GetCrewStars(HamProfile const *, Symbol, int &, int &) const;
    void
    GetCrewStarsForDifficulty(HamProfile const *, Symbol, Difficulty, int &, int &) const;
    int GetTotalNumLibrarySongs() const;
    void UploadSongLibraryToServer();
    void GetRankedSongs(std::vector<int> &) const;
    void GetRandomSelectableRankedSongs(std::vector<int> &) const;
    Symbol GetRandomSong();
    void InitializePlaylists();
    void GetValidSongs(class MetaPerformer const &, std::vector<Symbol> &) const;
    const char *MidiFile(Symbol) const;
    bool ToggleRandomSongDebug();
    const std::vector<int> &RankedSongs(SongType) const;
    void GetRandomlySelectableRankedSongs(std::vector<int> &) const;

    int GetNumPlaylists() const { return mPlaylists.size(); }

private:
    DataNode OnGetRandomSong(DataArray *);

protected:
    virtual void AddSongData(DataArray *, DataLoader *, ContentLocT);
    virtual void AddSongData(
        DataArray *,
        std::map<int, SongMetadata *> &,
        const char *,
        ContentLocT,
        std::vector<int> &
    );
    virtual void AddSongIDMapping(int, Symbol);
    virtual void ReadCachedMetadataFromStream(BinStream &, int);
    virtual void WriteCachedMetadataToStream(BinStream &) const;

    void ClearPlaylists();

    DataArraySongInfo *unkd0; // 0xd0
    std::map<int, Symbol> mSongNameLookup; // 0xd4
    std::map<Symbol, int> mSongIDLookup; // 0xec
    std::map<int, Symbol> mExtraSongIDMap; // 0x104
    std::vector<int> unk11c; // 0x11c - ranked?
    std::vector<int> unk128; // 0x128 - disc songs?
    std::vector<std::pair<int, int> > mRankTiers; // 0x134
    Jukebox mJukebox; // 0x140
    std::vector<String> mContentAltDirs; // 0x150
    std::vector<Playlist *> mPlaylists; // 0x15c
    bool mRandomSongDebug; // 0x168
};

extern HamSongMgr TheHamSongMgr;
