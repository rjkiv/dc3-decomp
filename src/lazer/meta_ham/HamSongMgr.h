#pragma once
#include "HamSongMetadata.h"
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Playlist.h"
#include "meta/Jukebox.h"
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

class HamSongMgr : public SongMgr {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual void ContentMounted(char const *, char const *);
    virtual char const *ContentPattern();
    virtual char const *ContentDir();
    virtual HamSongMetadata const *Data(int) const;
    virtual bool HasContentAltDirs();
    virtual SongInfo *SongAudioData(int) const;
    virtual Symbol GetShortNameFromSongID(int, bool) const;
    virtual int GetSongIDFromShortName(Symbol, bool) const;
    virtual void Terminate();
    virtual void Init();
    virtual void ContentDone();

    HamSongMgr();
    char const *GetAlbumArtPath(Symbol) const;
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
    int RankTier(int) const;
    int RankTier(Symbol) const;
    void
    GetCoreStarsForDifficulty(class HamProfile const *, Difficulty, int &, int &) const;
    void GetCharacterStars(class HamProfile const *, Symbol, int &, int &) const;
    void GetCrewStars(class HamProfile const *, Symbol, int &, int &) const;
    void GetCrewStarsForDifficulty(
        class HamProfile const *, Symbol, Difficulty, int &, int &
    ) const;
    int GetTotalNumLibrarySongs() const;
    void UploadSongLibraryToServer();
    void GetRankedSongs(std::vector<int> &) const;
    void GetRandomSelectableRankedSongs(std::vector<int> &) const;
    Symbol GetRandomSong();
    void InitializePlaylists();
    void GetValidSongs(class MetaPerformer const &, std::vector<Symbol> &) const;

    int *unkd0;
    std::map<int, Symbol> unkd4;
    std::map<Symbol, int> unkec;
    std::map<int, Symbol> unk104;
    std::vector<int> unk11c;
    std::vector<int> unk128;
    std::vector<struct stlpmtx_std::pair<int, int> > unk134;
    Jukebox mJukebox; // 0x140
    std::vector<String> unk150;
    std::vector<Playlist *> mPlaylists; // 0x15c
    bool unk168;

protected:
    virtual void WriteCachedMetadataToStream(BinStream &) const;
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

    void ClearPlaylists();

private:
    DataNode OnGetRandomSong(DataArray *);
};

extern HamSongMgr TheHamSongMgr;
