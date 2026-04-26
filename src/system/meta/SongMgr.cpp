#include "SongMgr.h"
#include "SongMetadata.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "stl/_vector.h"
#include "utl/BufStream.h"
#include "utl/Cache.h"
#include "utl/CacheMgr.h"
#include "utl/MemMgr.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include <set>
#include <vector>
#include <map>

const char *SONG_CACHE_CONTAINER_NAME = "songcache";
int gSongCacheSaveVer = 12;

namespace {
    const char *gStrSongMgrState[] = { "kSongMgr_SaveMount",
                                       "kSongMgr_SaveWrite",
                                       "kSongMgr_SaveUnmount",
                                       "kSongMgr_Ready",
                                       "kSongMgr_Failure" };
}

int GetSongID(DataArray *main_arr, DataArray *backup_arr) {
    static Symbol song_id("song_id");
    int songID = 0;
    main_arr->FindData(song_id, songID, false);
    if (songID == 0 && backup_arr) {
        backup_arr->FindData(song_id, songID, false);
    }
    return songID;
}

int CountSongsInArray(DataArray *arr) {
    int i = 0;
    int size = arr->Size();
    for (; i < size && arr->Type(i) != kDataArray; i++)
        ;
    return size - i;
}

#pragma region Hmx::Object

SongMgr::~SongMgr() {}

BEGIN_HANDLERS(SongMgr)
    HANDLE_EXPR(content_name, Symbol(ContentName(_msg->Int(2))))
    HANDLE_EXPR(content_name_from_sym, Symbol(ContentName(_msg->Sym(2), true)))
    HANDLE_EXPR(is_song_mounted, IsSongMounted(_msg->Sym(2)))
    HANDLE_ACTION(clear_from_cache, ClearFromCache(_msg->Sym(2)))
    HANDLE_EXPR(get_total_num_songs, (int)mAvailableSongs.size())
    HANDLE_EXPR(get_song_id_from_short_name, GetSongIDFromShortName(_msg->Sym(2), true))
    HANDLE_EXPR(data, (Hmx::Object *)Data(_msg->Int(2)))
    HANDLE_ACTION(cache_mgr_mount_result, OnCacheMountResult(_msg->Int(2)))
    HANDLE_ACTION(cache_write_result, OnCacheWriteResult(_msg->Int(2)))
    HANDLE_ACTION(cache_mgr_unmount_result, OnCacheUnmountResult(_msg->Int(2)))
    HANDLE_EXPR(num_songs_in_content, NumSongsInContent(_msg->Sym(2)))
    HANDLE_ACTION(dump_songs, DumpSongMgrContents(false))
    HANDLE_ACTION(dump_all_songs, DumpSongMgrContents(true))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion
#pragma region ContentMgr::Callback

void SongMgr::ContentStarted() {
    mAvailableSongs.clear();
    FOREACH (it, mCachedSongMetadata) {
        it->second->IncrementAge();
    }
    mContentUsedForSong.clear();
}

bool SongMgr::ContentDiscovered(Symbol contentName) {
    if (HasContent(contentName)) {
        std::vector<int> songs;
        GetSongsInContent(contentName, songs);
        FOREACH (it, songs) {
            int songID = *it;
            auto found = mCachedSongMetadata.find(songID);
            if (found != mCachedSongMetadata.end()) {
                found->second->ResetAge();
                if (!HasSong(songID)) {
                    mAvailableSongs.insert(songID);
                    mContentUsedForSong[songID] = contentName;
                    AddSongIDMapping(songID, Data(songID)->ShortName());
                }
            }
        }
        return true;
    } else
        return false;
}

void SongMgr::ContentMounted(const char *name, const char *cc2) {
    unkmap5[name] = cc2;
    if (!HasContent(name)) {
        std::vector<int> songs;
        mSongIDsInContent[name] = songs;
    }
}

void SongMgr::ContentUnmounted(char const *cc) {
    auto it = unkmap5.find(Symbol(cc));
    if (it != unkmap5.end()) {
        unkmap5.erase(it);
    }
}

void SongMgr::ContentLoaded(Loader *loader, ContentLocT location, Symbol contentName) {
    DataLoader *d = dynamic_cast<DataLoader *>(loader);
    MILO_ASSERT(d, 0xDC);
    DataArray *data = d->Data();
    if (data) {
        if (AllowContentToBeAdded(data, location)) {
            if (!streq(contentName.Str(), ".")) {
                CacheSongData(data, d, location, contentName);
            } else {
                AddSongData(data, d, location);
            }
        } else {
            std::vector<int> somevecidk;
            int datasize = data->Size();
            for (int i = datasize - CountSongsInArray(data); i < datasize; i++) {
                int songID = GetSongID(data->Array(i), nullptr);
                auto idIt = mAvailableSongs.find(songID);
                if (idIt != mAvailableSongs.end()) {
                    mAvailableSongs.erase(idIt);
                }
                auto contentIt = mContentUsedForSong.find(songID);
                if (contentIt != mContentUsedForSong.end()) {
                    mContentUsedForSong.erase(contentIt);
                }
            }
            ClearFromCache(contentName);
        }
    } else
        ClearFromCache(contentName);
}

void SongMgr::ContentDone() {
    if (unkcc)
        mSongCacheNeedsWrite = true;
}

#pragma endregion
#pragma region SongMgr

void SongMgr::Init() {
    mState = kSongMgr_Nil;
    mSongCacheID = nullptr;
    mSongCache = nullptr;
    unkcc = false;
    mSongCacheNeedsWrite = false;
}

const SongMetadata *SongMgr::Data(int songID) const {
    if (!HasSong(songID))
        return nullptr;
    else {
        auto it = mUncachedSongMetadata.find(songID);
        if (it != mUncachedSongMetadata.end())
            return it->second;
        else {
            auto cit = mCachedSongMetadata.find(songID);
            if (cit != mCachedSongMetadata.end())
                return cit->second;
            else {
                MILO_ASSERT(false, 0x8B);
                return nullptr;
            }
        }
    }
}

void SongMgr::GetContentNames(Symbol shortname, std::vector<Symbol> &names) const {
    const char *cntName = ContentName(shortname, false);
    if (cntName) {
        names.push_back(cntName);
    }
}

void SongMgr::ClearCachedContent() {
    mSongIDsInContent.clear();
    FOREACH (it, mCachedSongMetadata) {
        delete it->second;
    }
    mCachedSongMetadata.clear();
}

SongInfo *SongMgr::SongAudioData(Symbol shortname) const {
    return SongAudioData(GetSongIDFromShortName(shortname, true));
}

bool SongMgr::IsSongCacheWriteDone() const {
    return mState == kSongMgr_Ready || mState == kSongMgr_Failure;
}

char const *SongMgr::GetCachedSongInfoName() const { return SONG_CACHE_CONTAINER_NAME; }

char const *SongMgr::SongPath(Symbol shortname, int version) const {
    const char *filename = SongAudioData(shortname)->GetBaseFileName();
    if (version != 0) {
        int songID = GetSongIDFromShortName(shortname, true);
        if (Data(songID)->Version() < version) {
            const char *base = FileGetBase(filename);
            filename = MakeString("%s%s/%s", AlternateSongDir(), base, base);
        }
    }
    return filename;
}

char const *SongMgr::SongFilePath(Symbol shortname, char const *path, int version) const {
    const char *pathstr = MakeString("%s%s", SongPath(shortname, version), path);
    return CachedPath(shortname, pathstr, version);
}

void SongMgr::DumpSongMgrContents(bool all) {
    MILO_LOG("--------------------\n");
    int idx = 0;
    int skipped = 0;
    MILO_LOG("mAvailableSongs:\n");
    FOREACH (it, mAvailableSongs) {
        if (all || *it > 1000000) {
            MILO_LOG(" %d. ID: %d\n", idx, *it);
        } else
            skipped++;
        idx++;
    }
    if (skipped > 0) {
        MILO_LOG(" skipped %d non-DLC songs\n", skipped);
    }
    MILO_LOG(" Total Count: %d\n", idx);
    idx = 0;
    skipped = 0;
    MILO_LOG("mUncachedSongMetadata:\n");
    FOREACH (it, mUncachedSongMetadata) {
        SongMetadata *meta = it->second;
        int id = meta->ID();
        if (all || id > 1000000) {
            MILO_LOG(
                " %d. ID: %d, Short Name: %s, Age: %d\n",
                idx,
                meta->ID(),
                meta->ShortName(),
                meta->Age()
            );
        } else
            skipped++;
        idx++;
    }
    if (skipped > 0) {
        MILO_LOG(" skipped %d non-DLC songs\n", skipped);
    }
    MILO_LOG(" Total Count: %d\n", idx);
    idx = 0;
    MILO_LOG("mCachedSongMetadata:\n");
    FOREACH (it, mCachedSongMetadata) {
        SongMetadata *meta = it->second;
        MILO_LOG(
            " %d. ID: %d, Short Name: %s, Age: %d\n",
            idx,
            meta->ID(),
            meta->ShortName(),
            meta->Age()
        );
        idx++;
    }
    MILO_LOG(" Total Count: %d\n", idx);
    idx = 0;
    MILO_LOG("mSongIDsInContent:\n");
    FOREACH (it, mSongIDsInContent) {
        MILO_LOG(" %d. Content: %s\n", idx, it->first);
        idx++;
        std::vector<int> &songIDs = it->second;
        FOREACH (intIt, songIDs) {
            MILO_LOG("  SongID: %d\n", *intIt);
        }
    }
    MILO_LOG(" Total Count: %d\n", idx);
    idx = 0;
    MILO_LOG("mContentUsedForSong:\n");
    FOREACH (it, mContentUsedForSong) {
        MILO_LOG(" %d. ID: %d, Content: %s\n", idx, it->first, it->second);
        // i think they forgot to increment idx per iteration here lol
    }
    MILO_LOG(" Total Count: %d\n", idx);
    MILO_LOG("--------------------\n");
}

bool SongMgr::HasSong(int songID) const {
    return mAvailableSongs.find(songID) != mAvailableSongs.end();
}

bool SongMgr::HasSong(Symbol shortname, bool fail) const {
    int songid = GetSongIDFromShortName(shortname, fail);
    return songid != 0 && HasSong(songid);
}

const char *SongMgr::ContentName(int songID) const {
    const SongMetadata *data = Data(songID);
    if (data && !data->IsOnDisc()) {
        auto it = mContentUsedForSong.find(songID);
        MILO_ASSERT(it != mContentUsedForSong.end(), 0x158);
        return it->second.Str();
    } else
        return nullptr;
}

int SongMgr::GetCachedSongInfoSize() const {
    MemStream ms;
    ms << 0; // rev
    ms << mSongIDsInContent;
    WriteCachedMetadataToStream(ms);
    return ms.Tell();
}

const char *SongMgr::CachedPath(Symbol shortname, const char *cc, int version) const {
    int songID = GetSongIDFromShortName(shortname);
    const SongMetadata *data = Data(songID);
    if (!UsingCD()
        && (!data || (!data->IsOnDisc() && (version == 0 || data->Version() >= version)))
        && strstr(cc, ".milo")) {
        DirLoader::SetCacheMode(true);
        cc = DirLoader::CachedPath(cc, false);
        DirLoader::SetCacheMode(false);
    }
    return cc;
}

bool SongMgr::IsSongMounted(Symbol shortname) const {
    const char *name = ContentName(GetSongIDFromShortName(shortname, true));
    if (name) {
        return TheContentMgr.IsMounted(name);
    } else
        return true;
}

bool SongMgr::SaveCachedSongInfo(BufStream &bs) {
    bs << gSongCacheSaveVer;
    bs << mSongIDsInContent;
    WriteCachedMetadataToStream(bs);
    return true;
}

bool SongMgr::IsContentUsedForSong(Symbol contentName, int songID) const {
    auto it = mContentUsedForSong.find(songID);
    return it != mContentUsedForSong.end() && it->second == contentName;
}

void SongMgr::StartSongCacheWrite() {
    if (SongCacheNeedsWrite()) {
        ClearSongCacheNeedsWrite();
        if (mState == kSongMgr_SaveUnmount && mSongCache) {
            SetState(kSongMgr_SaveWrite);
        } else
            SetState(kSongMgr_SaveMount);
    }
}

void SongMgr::ClearFromCache(Symbol contentName) {
    auto it = mSongIDsInContent.find(contentName);
    MILO_ASSERT_FMT(
        it != mSongIDsInContent.end(), "Content %s isn't cached!", contentName
    );
    mSongIDsInContent.erase(it);
}

const char *SongMgr::ContentName(Symbol shortname, bool fail) const {
    return ContentName(GetSongIDFromShortName(shortname, fail));
}

bool SongMgr::LoadCachedSongInfo(BufStream &bs) {
    ClearCachedContent();
    int rev;
    bs >> rev;
    bs >> mSongIDsInContent;
    ReadCachedMetadataFromStream(bs, rev);
    if (rev < gSongCacheSaveVer) {
        ClearCachedContent();
    }
    return true;
}

void SongMgr::SaveMount() {
    if (!mSongCacheID) {
        mSongCacheID = TheCacheMgr->GetCacheID(SONG_CACHE_CONTAINER_NAME);
    }
    if (mSongCacheID) {
        if (TheCacheMgr->MountAsync(mSongCacheID, &mSongCache, this))
            return;
        CacheResult res = TheCacheMgr->GetLastResult();
        if (res != kCache_ErrorBusy) {
            MILO_FAIL("SongMgr: Error %d while mounting.\n", res);
            return;
        }
    }
    SetState(kSongMgr_Ready);
}

void SongMgr::SaveUnmount() {
    if (mSongCache) {
        if (!TheCacheMgr->UnmountAsync(&mSongCache, this)) {
            CacheResult res = TheCacheMgr->GetLastResult();
            if (res != kCache_ErrorBusy) {
                MILO_FAIL("SongMgr: Error %d while unmounting.\n", res);
            }
        } else {
            mSongCache = nullptr;
        }
    } else {
        MILO_LOG("SongMgr: Failed to unmount NULL song info cache.\n");
    }
}

void SongMgr::SaveWrite() {
    int size = GetCachedSongInfoSize();
    void *tmp = _MemAllocTemp(size, __FILE__, 0x2DD, "SongMgr", 0);
    BufStream bs(tmp, size, true);
    if (SaveCachedSongInfo(bs)) {
        bool ret = mSongCache->WriteAsync(SONG_CACHE_CONTAINER_NAME, tmp, size, this);
        MILO_ASSERT(ret, 0x2E3);
    } else {
        MILO_LOG("SongMgr: Failed to save cached song info - write aborted.\n");
        SetState(kSongMgr_Ready);
    }
}

void SongMgr::GetSongsInContent(Symbol contentName, std::vector<int> &songIDs) const {
    auto it = mSongIDsInContent.find(contentName);
    if (it != mSongIDsInContent.end())
        songIDs = it->second;
}

char const *SongMgr::ContentNameRoot(Symbol contentName) const {
    auto it = unkmap5.find(contentName);
    if (it == unkmap5.end())
        return nullptr;
    else
        return it->second.c_str();
}

int SongMgr::NumSongsInContent(Symbol contentName) const {
    auto it = mSongIDsInContent.find(contentName);
    if (it != mSongIDsInContent.end())
        return it->second.size();
    else
        return 0;
}

void SongMgr::SetState(SongMgrState state) {
    if (mState == state)
        return;
    mState = state;
    switch (mState) {
    case kSongMgr_SaveMount:
        SaveMount();
        break;
    case kSongMgr_SaveWrite:
        SaveWrite();
        break;
    case kSongMgr_SaveUnmount:
        SaveUnmount();
        break;
    default:
        break;
    }
}

void SongMgr::CacheSongData(
    DataArray *arr, DataLoader *loader, ContentLocT location, Symbol contentName
) {
    std::vector<int> contentSongs;
    GetSongsInContent(contentName, contentSongs);
    if (!contentSongs.empty())
        return;
    else {
        std::vector<int> otherIntVec;
        AddSongData(arr, mCachedSongMetadata, ".", location, otherIntVec);
        std::vector<int> songIDs;
        for (int i = 0; i < arr->Size(); i++) {
            Symbol curSym = arr->Array(i)->Sym(0);
            int songID = GetSongIDFromShortName(curSym, false);
            if (songID != 0)
                songIDs.push_back(songID);
        }
        mSongIDsInContent[contentName] = songIDs;
        FOREACH (it, otherIntVec) {
            int id = *it;
            MILO_ASSERT(mContentUsedForSong.find(id) == mContentUsedForSong.end(), 0x2AF);
            mContentUsedForSong[id] = contentName;
        }
        unkcc = true;
    }
}

void SongMgr::OnCacheMountResult(int res) {
    if (mState != kSongMgr_SaveMount) {
        MILO_LOG("SongMgr: Mount result received in state %d.\n", mState);
    } else if (res != 0) {
        MILO_LOG("SongMgr: Mount result error %d - aborting cache write.\n", res);
        SetState(kSongMgr_Ready);
    } else
        SetState(kSongMgr_SaveWrite);
}

void SongMgr::OnCacheWriteResult(int res) {
    if (mState != kSongMgr_SaveWrite) {
        MILO_LOG("SongMgr: Write result received in state %d.\n", mState);
    } else {
        if (res != 0)
            MILO_LOG("SongMgr: Write result error %d - cache write failed.\n", res);
        SetState(kSongMgr_SaveUnmount);
    }
}

void SongMgr::OnCacheUnmountResult(int res) {
    if (mState != kSongMgr_SaveUnmount) {
        MILO_LOG("SongMgr: Unmount result received in state %d.\n", mState);
    } else {
        if (res != 0)
            MILO_LOG("SongMgr: Unmount result error %d - aborting cache unmount.\n", res);
        unkcc = false;
        SetState(kSongMgr_Ready);
    }
}

const std::set<int> &SongMgr::GetAvailableSongSet() const { return mAvailableSongs; }
