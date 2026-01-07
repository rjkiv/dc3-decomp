#pragma once
#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "utl/BufStream.h"
#include "utl/Cache.h"
#include "utl/MemStream.h"
#include "utl/SongInfoCopy.h"
#include <set>
#include <vector>
#include <map>

// from RB2 taken from RB3 decomp
enum SongMgrState {
    kSongMgr_SaveMount = 0,
    kSongMgr_SaveWrite = 1,
    kSongMgr_SaveUnmount = 2,
    kSongMgr_Ready = 3,
    kSongMgr_Failure = 4,
    kSongMgr_Max = 5,
    kSongMgr_Nil = -1,
};

enum SongID {
    kSongID_Invalid = 0,
    kSongID_Any = -1,
    kSongID_Random = -2
};

class SongMgr : public Hmx::Object, public ContentMgr::Callback {
public:
    SongMgr() {}
    // Hmx::Object
    virtual ~SongMgr();
    virtual DataNode Handle(DataArray *, bool);

    // ContentMgr::Callback
    virtual void ContentStarted();
    virtual bool ContentDiscovered(Symbol);
    virtual void ContentMounted(char const *, char const *);
    virtual void ContentUnmounted(char const *);
    virtual void ContentLoaded(Loader *, ContentLocT, Symbol);
    virtual void ContentDone();

    // SongMgr
    virtual void Init();
    virtual void Terminate() {}
    /** Get the song metadata associated with the supplied song ID.
     * @param [in] songID The song ID.
     * @returns The corresponding song metadata.
     */
    virtual const SongMetadata *Data(int songID) const; // 0x60
    /** Get the song audio data associated with the supplied song ID. */
    virtual SongInfo *SongAudioData(int songID) const = 0;
    virtual char const *AlternateSongDir() const { return "songs/updates/"; }
    /** Add a song's content name to the given vector of names.
     * @param [in] shortname The song's shortname.
     * @param [out] names The collection of song content names.
     */
    virtual void GetContentNames(Symbol shortname, std::vector<Symbol> &names) const;
    virtual bool SongCacheNeedsWrite() const { return mSongCacheNeedsWrite; }
    virtual void ClearSongCacheNeedsWrite() { mSongCacheNeedsWrite = false; }
    virtual void ClearCachedContent();
    /** Get the song shortname associated with the supplied song ID.
     * @param [in] songID The song ID.
     * @param [in] fail If true, and the song can't be found, fail the system.
     * @returns The corresponding song shortname.
     */
    virtual Symbol GetShortNameFromSongID(int songID, bool fail = true) const = 0;
    /** Get the song ID associated with the supplied song shortname.
     * @param [in] shortname The song shortname.
     * @param [in] fail If true, and the song can't be found, fail the system.
     * @returns The corresponding song ID.
     */
    virtual int GetSongIDFromShortName(Symbol shortname, bool fail = true) const = 0;

    /** Get the song audio data associated with the supplied shortname. */
    SongInfo *SongAudioData(Symbol shortname) const;
    bool IsSongCacheWriteDone() const;
    char const *GetCachedSongInfoName() const;
    char const *SongPath(Symbol shortname, int version) const;
    char const *SongFilePath(Symbol, char const *, int) const;
    /** Dump the contents of the SongMgr to the console.
     * @param [in] all If true, print all the songs we have. Else, skip non-DLC songs.
     */
    void DumpSongMgrContents(bool all);
    /** Do we have the supplied songID in our list of available songs? */
    bool HasSong(int songID) const;
    /** Do we have the supplied shortname in our list of available songs? */
    bool HasSong(Symbol shortname, bool fail = true) const;
    int GetCachedSongInfoSize() const;
    bool IsSongMounted(Symbol shortname) const;
    bool SaveCachedSongInfo(BufStream &);
    /** Does the supplied content file name house the supplied song ID? */
    bool IsContentUsedForSong(Symbol contentName, int songID) const;
    void StartSongCacheWrite();
    /** Remove the supplied content file name from the cache. */
    void ClearFromCache(Symbol contentName);
    /** Given a songID, get the name of the content file it comes from. */
    const char *ContentName(int songID) const;
    /** Given a shortname, get the name of the content file it comes from. */
    const char *ContentName(Symbol shortname, bool fail = true) const;
    bool LoadCachedSongInfo(BufStream &);

    /** Do we have this content file name in our records? */
    bool HasContent(Symbol contentName) {
        return mSongIDsInContent.find(contentName) != mSongIDsInContent.end();
    }

protected:
    virtual bool AllowContentToBeAdded(DataArray *, ContentLocT) { return true; }
    virtual void AddSongData(DataArray *, DataLoader *, ContentLocT) = 0;
    virtual void AddSongData(
        DataArray *,
        std::map<int, SongMetadata *> &,
        const char *,
        ContentLocT,
        std::vector<int> &
    ) = 0;
    virtual void AddSongIDMapping(int, Symbol) = 0;
    virtual void ReadCachedMetadataFromStream(BinStream &, int) = 0;
    virtual void WriteCachedMetadataToStream(BinStream &) const = 0;

    char const *CachedPath(Symbol shortname, const char *, int version) const;
    void SaveMount();
    void SaveUnmount();
    void SaveWrite();
    /** Given a content file name, get the file's song IDs. */
    void GetSongsInContent(Symbol contentName, std::vector<int> &songIDs) const;
    char const *ContentNameRoot(Symbol contentName) const;
    int NumSongsInContent(Symbol contentName) const;
    void SetState(SongMgrState state);
    void OnCacheMountResult(int result);
    void OnCacheWriteResult(int result);
    void OnCacheUnmountResult(int result);
    void CacheSongData(
        DataArray *, DataLoader *loader, ContentLocT location, Symbol contentName
    );

    /** The available songs we can select in-game. Key = song ID */
    std::set<int> mAvailableSongs; // 0x30
    std::map<int, SongMetadata *> mUncachedSongMetadata; // 0x48
    /** The current state of the SongMgr. */
    SongMgrState mState; // 0x60
    std::map<int, SongMetadata *> mCachedSongMetadata; // 0x64
    /** A collection of content files (CON/LIVES), and the song IDs inside each file.
        Key = content file name (i.e. RBMEGAPACK01OF10); Value = the song IDs.
    */
    std::map<Symbol, std::vector<int> > mSongIDsInContent; // 0x7c
    /** A collection of song IDs, and the contents they came from.
        Key = song ID;
        Value = the content file name (i.e. RBMEGAPACK01OF10) that houses this song
    */
    std::map<int, Symbol> mContentUsedForSong; // 0x94
    // key = content file name. value = root name???
    std::map<Symbol, String> unkmap5; // 0xac - mounted content?
    CacheID *mSongCacheID; // 0xc4
    Cache *mSongCache; // 0xc8
    bool unkcc; // 0xcc
    bool mSongCacheNeedsWrite; // 0xcd
    bool mSongCacheWriteAllowed; // 0xce
};

extern SongMgr &TheSongMgr;
