#include "lazer/meta_ham/HamSongMgr.h"
#include "HamSongMetadata.h"
#include "lazer/meta_ham/Playlist.h"
#include "macros.h"
#include "meta/Jukebox.h"
#include "meta/SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "stl/_vector.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/SongInfoCopy.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

HamSongMgr TheHamSongMgr;

HamSongMgr::HamSongMgr() : unkd0(0), mJukebox(Jukebox(2000)), unk168(false) {
    ClearAndShrink<String>(unk150);
}

void HamSongMgr::ContentMounted(char const *c1, char const *c2) {
    SongMgr::ContentMounted(c1, c2);
}

char const *HamSongMgr::ContentPattern() { return "songs*.dta"; }

char const *HamSongMgr::ContentDir() { return "songs"; }

HamSongMetadata const *HamSongMgr::Data(int) const { return nullptr; }

bool HamSongMgr::HasContentAltDirs() { return unk128.size() != 0; }

SongInfo *HamSongMgr::SongAudioData(int) const { return nullptr; }

Symbol HamSongMgr::GetShortNameFromSongID(int, bool) const { return 0; }

int HamSongMgr::GetSongIDFromShortName(Symbol s, bool b) const { return 1; }

void HamSongMgr::Terminate() {
    RELEASE(unkd0);
    unkd4.clear();
    unkec.clear();
    TheContentMgr.UnregisterCallback(this, false);
    ClearAndShrink<String>(unk150);
    ClearPlaylists();
}

void HamSongMgr::Init() {}

void HamSongMgr::ContentDone() {}

char const *HamSongMgr::GetAlbumArtPath(Symbol s) const {
    if (SongMgr::HasSong(s, true)) {
        return SongMgr::SongFilePath(s, "_keep.png", 0);
    }
    return gNullStr;
}

bool HamSongMgr::IsDummySong(Symbol) const { return false; }

void HamSongMgr::AddSongs(DataArray *) {}

void HamSongMgr::AddRecentSong(Symbol s) {
    int id = GetSongIDFromShortName(s, true);
    mJukebox.Play(id);
}

Symbol HamSongMgr::GetArtistNameFromShortName(Symbol s) {
    int id = GetSongIDFromShortName(s, true);
    const HamSongMetadata *meta = Data(id);
    char const *artist = meta->Artist(); // so what was the point of this
    return meta->Artist();
}

Playlist *HamSongMgr::GetPlaylist(Symbol p) {
    FOREACH (it, mPlaylists) {
        Playlist *pPlaylist = *it;
        MILO_ASSERT(pPlaylist, 0xa4);
        if (pPlaylist->unk4 == p) {
            return pPlaylist;
        }
    }
    return nullptr;
}

Playlist *HamSongMgr::GetPlaylist(int index) {
    MILO_ASSERT((0) <= (index) && (index) < (mPlaylists.size()), 0xc2);
    return mPlaylists[index];
}

Playlist *HamSongMgr::GetPlaylistWithLocalizedName(String p) {
    FOREACH (it, mPlaylists) {
        Playlist *playlist = *it;
        MILO_ASSERT(playlist, 0xb5);
        const char *locName = Localize(playlist->unk4, nullptr, TheLocale);
        if (locName == p.c_str()) {
            return playlist;
        }
    }
    return nullptr;
}

char const *HamSongMgr::BarksFile(Symbol song) const {
    static Symbol nor("nor");
    Symbol lang = SystemLanguage();
    int ver = 10;
    if (lang == nor) {
        ver = 0xb;
    }

    const char *songpath = SongMgr::SongPath(song, ver);
    const char *filepath = FileGetPath(songpath);
    songpath = MakeString("%s/loc/%s/barks.milo", filepath, lang);

    return SongMgr::CachedPath(song, songpath, ver);
}

int HamSongMgr::GetDuration(Symbol song) const {
    int id = GetSongIDFromShortName(song, true);
    const HamSongMetadata *data = Data(id);
    MILO_ASSERT(data, 0x1f6);
    int length = data->LengthMs();
    return length / 1000;
}

Symbol HamSongMgr::GetCharacter(Symbol song) const {
    int id = GetSongIDFromShortName(song, true);
    const HamSongMetadata *data = Data(id);
    MILO_ASSERT(data, 0x1fe);
    Symbol character = data->Character();
    return character;
}

int HamSongMgr::GetBPM(Symbol song) const {
    int id = GetSongIDFromShortName(song, true);
    const HamSongMetadata *data = Data(id);
    MILO_ASSERT(data, 0x207);
    int bpm = data->Bpm();
    return bpm;
}

Symbol HamSongMgr::RankTierToken(int token) const {
    return MakeString("song_tier_%i", token);
}

int HamSongMgr::RankTier(int) const { return 1; }

int HamSongMgr::RankTier(Symbol) const { return 1; }

int HamSongMgr::GetTotalNumLibrarySongs() const { return 1; }

void HamSongMgr::UploadSongLibraryToServer() {}

void HamSongMgr::GetRankedSongs(std::vector<int> &) const {}

void HamSongMgr::GetRandomSelectableRankedSongs(std::vector<int> &) const {}

Symbol HamSongMgr::GetRandomSong() { return 0; }

void HamSongMgr::InitializePlaylists() {
    ClearPlaylists();
    static Symbol playlists("playlists");
    static Symbol songs("songs");
    DataArray *playlistArray = SystemConfig(playlists);
    MILO_ASSERT(playlistArray, 0xd9);

    if (1 < playlistArray->Size()) {
        for (int i = 1; i < playlistArray->Size(); i++) {
            DataArray *playlistEntry = playlistArray->Node(i).Array();
            MILO_ASSERT(playlistEntry, 0xdf);

            Symbol s = playlistEntry->Sym(0);
            Playlist *p = new Playlist();

            static Symbol is_fitness("is_fitness");
            p->unk4 = s;
            p->unk8 = false;
            DataArray *songArray = playlistEntry->FindArray(songs, true);
            MILO_ASSERT(songArray, 0xed);

            if (1 < songArray->Size()) {
                for (int i = 1; i < songArray->Size(); i++) {
                    Symbol sym = songArray->Sym(i);
                    int songID = GetSongIDFromShortName(sym, 0);
                    if (songID == 0) {
                        MILO_NOTIFY(
                            "HMX Playlist: %s is referring to unknown song: %s",
                            sym,
                            songID
                        );
                    } else {
                        p->AddSong(songID);
                    }
                }
            }
            if (!p->m_vSongs.empty()) {
                mPlaylists.push_back(p);
            }
        }
    }
    char buffer[16] = {};
}

void HamSongMgr::WriteCachedMetadataToStream(BinStream &) const {}

void HamSongMgr::AddSongData(DataArray *d, DataLoader *dl, ContentLocT clt) {}

void HamSongMgr::AddSongData(
    DataArray *,
    std::map<int, SongMetadata *> &,
    const char *,
    ContentLocT,
    std::vector<int> &
) {}

void HamSongMgr::AddSongIDMapping(int, Symbol) {}

void HamSongMgr::ReadCachedMetadataFromStream(BinStream &, int) {}

void HamSongMgr::ClearPlaylists() {
    FOREACH (it, mPlaylists) {
        delete (*it);
        *it = nullptr;
    }
    mPlaylists.clear();
}

DataNode HamSongMgr::OnGetRandomSong(DataArray *d) { return NULL_OBJ; }

BEGIN_HANDLERS(HamSongMgr)
END_HANDLERS
