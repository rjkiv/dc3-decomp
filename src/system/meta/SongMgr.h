#pragma once
#include "meta/SongMetadata.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "utl/BufStream.h"
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

class SongMgr : public MemStream, public ContentMgr::Callback, public Hmx::Object{

    public:
        virtual void Init();
        virtual void ContentDone();
        SongInfo* SongAudioData(Symbol) const;
        bool IsSongCacheWriteDone() const;
        char const * GetCachedSongInfoName() const;
        virtual void ClearSongCacheNeedsWrite();
        char const * SongPath(Symbol, int) const;
        char const * SongFilePath(Symbol, char const *, int) const;
        void DumpSongMgrContents(bool);
        bool HasSong(int) const;
        bool HasSong(Symbol, bool) const;
        int GetCachedSongInfoSize(void) const;
        bool IsSongMounted(Symbol) const;
        bool SaveCachedSongInfo(BufStream &);
        bool IsContentUsedForSong(Symbol, int) const;
        virtual SongMetadata const * Data(int) const;
        virtual void ContentStarted();
        virtual void ContentUnmounted(char const *);
        void StartSongCacheWrite();
        virtual bool ContentDiscovered(Symbol);
        void ClearFromCache(Symbol);
        virtual DataNode Handle(DataArray *, bool);
        virtual void ClearCachedContent(void);
        virtual ~SongMgr();
        virtual char const * AlternateSongDir() const;
        virtual void ContentMounted(char const *, char const *);
        virtual void GetContentNames(Symbol, std::vector<Symbol> &) const;
        bool LoadCachedSongInfo(BufStream &);
        virtual void WriteCachedMetadataFromStream(BinStream &) const = 0;
        virtual int GetSongIDFromShortName(Symbol, bool) const = 0;

        std::set<int> mAvailableSongs; 
        std::map<int, SongMetadata *> mUncachedSongMetadata; 
        SongMgrState mState; 
        std::map<int, SongMetadata *> mCachedSongMetadata; 
        std::map<Symbol, std::vector<int> > mSongIDsInContent; 
        std::map<int, Symbol> mContentUsedForSong; 
        std::map<Symbol, String> unkmap5; 
        //CacheID *mSongCacheID; 
        //Cache *mSongCache; 
        bool unkbc; 
        bool mSongCacheNeedsWrite; 
        bool mSongCacheWriteAllowed; 

    protected:
        char const * CachedPath(Symbol, char const *, int) const;
        void SaveMount();
        void SaveUnmount();
        void SaveWrite();
        void GetSongsInContent(Symbol, std::vector<int> &) const;
        char const * ContentNameRoot(Symbol) const;
        int NumSongsInContent(Symbol) const;
        void SetState(SongMgrState);
        void OnCacheMountResult(int);
        void OnCacheWriteResult(int);
        void OnCacheUnmountResult(int);
        void CacheSongData(DataArray *, DataLoader *, ContentLocT, Symbol);
        virtual void ContentLoaded(Loader *, ContentLocT, Symbol);

        
};

int GetSongID(DataArray *, DataArray *);
int CountSongsInArray(DataArray *);

extern SongMgr *TheBaseSongManager;
