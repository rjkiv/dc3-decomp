#include "SongMgr.h"
#include "SongMetadata.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "utl/BufStream.h"
#include "utl/MemMgr.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include <set>
#include <vector>
#include <map>

SongMgr *TheBaseSongManager;
const char *SONG_CACHE_CONTAINER_NAME = "songcache_bb";

int GetSongID(DataArray *main_arr, DataArray *backup_arr){
    static int sDebugSongID = 99000001;
    int theID = 0;
    return theID;
}

int CountSongsInArray(DataArray *arr){
    int i=0;
    int size = arr->Size();
    for (;i < size && arr->Node(i).Type() != kDataArray; i++)
        ;
    return size - i;
}

void SongMgr::Init(){
    mState = kSongMgr_Nil;
    //mSongCacheID = 0;
    //mSongCache = 0;
    unkbc = false;
    mSongCacheNeedsWrite = false;
    mSongCacheWriteAllowed = true;
}

void SongMgr::ContentDone(){
    if(!unkbc)
        return;
    mSongCacheNeedsWrite = true;
}

SongInfo* SongMgr::SongAudioData(Symbol s) const{
    //return SongAudioData(GetSongIDFromShortName(s, true));
    return nullptr;
}

bool SongMgr::IsSongCacheWriteDone() const{
    return mState == kSongMgr_Ready || mState == kSongMgr_Failure;
}

char const * SongMgr::GetCachedSongInfoName() const{
    return SONG_CACHE_CONTAINER_NAME;
}

void SongMgr::ClearSongCacheNeedsWrite(){
    mSongCacheNeedsWrite = false;
}

char const * SongMgr::SongPath(Symbol s, int i) const{
    SongInfo* info = SongAudioData(s);
    char const *c = "";
    if(i!=0){

    }
    return c;
}

char const * SongMgr::SongFilePath(Symbol s, char const * path, int i) const{
    return nullptr;
}

void SongMgr::DumpSongMgrContents(bool all){

}

bool SongMgr::HasSong(int id) const{
    return mAvailableSongs.find(id) != mAvailableSongs.end();
}

bool SongMgr::HasSong(Symbol s, bool b) const{
    int songid = GetSongIDFromShortName(s, b);
    bool ret = songid != 0;
    if(ret)
        ret = HasSong(songid);
    return ret;
}

int SongMgr::GetCachedSongInfoSize() const{
    MemStream ms(false);
    int rev = 0;
    ms << rev;
    ms << mSongIDsInContent;
    WriteCachedMetadataFromStream(ms);
    return ms.Tell();
}

bool SongMgr::IsSongMounted(Symbol s) const{
    
    return true;
}

bool SongMgr::SaveCachedSongInfo(BufStream &bs){
    return true;
}

bool SongMgr::IsContentUsedForSong(Symbol s, int i) const{
    return true;
}

SongMetadata const * SongMgr::Data(int) const{
    return nullptr;
}

void SongMgr::ContentStarted(){
    mAvailableSongs.clear();
    for (std::map<int, SongMetadata *>::iterator it = mCachedSongMetadata.begin();
         it != mCachedSongMetadata.end();
         ++it) {
        it->second->IncrementAge();
    }
    mContentUsedForSong.clear();
}

void SongMgr::ContentUnmounted(char const *cc){
    std::map<Symbol, String>::iterator it;
    it = unkmap5.find(cc);
    if (it != unkmap5.end()) {
        unkmap5.erase(it);
    }
}

void SongMgr::StartSongCacheWrite(){

}

bool SongMgr::ContentDiscovered(Symbol){
    return true;
}

void SongMgr::ClearFromCache(Symbol s){
    std::map<Symbol, std::vector<int> >::iterator it;
    it = mSongIDsInContent.find(s);
    MILO_ASSERT_FMT(it != mSongIDsInContent.end(), "Content %s isn't cached!", s);
    if (it != mSongIDsInContent.end()) {
        mSongIDsInContent.erase(it);
    }
}

void SongMgr::ClearCachedContent(){
    mSongIDsInContent.clear();
    for (std::map<int, SongMetadata *>::iterator it = mCachedSongMetadata.begin();
         it != mCachedSongMetadata.end();
         ++it) {
        delete it->second;
    }
    mCachedSongMetadata.clear();
}

SongMgr::~SongMgr(){

}

char const * SongMgr::AlternateSongDir() const{
    return nullptr;
}

void SongMgr::ContentMounted(char const *, char const *){

}

void SongMgr::GetContentNames(Symbol s, std::vector<Symbol> &vec) const{

}

bool SongMgr::LoadCachedSongInfo(BufStream &){
    return true;
}

char const * SongMgr::CachedPath(Symbol, char const *, int) const{
    return nullptr;
}

void SongMgr::SaveMount(){
    
}

void SongMgr::SaveUnmount(){

}

void SongMgr::SaveWrite(){
    
}

void SongMgr::GetSongsInContent(Symbol s, std::vector<int> &vec) const{
    std::map<Symbol, std::vector<int> >::const_iterator it = mSongIDsInContent.find(s);
    if (it != mSongIDsInContent.end())
        vec = it->second;
}

char const * SongMgr::ContentNameRoot(Symbol) const{
    return nullptr;
}

int SongMgr::NumSongsInContent(Symbol s) const{
    const char * c = ContentNameRoot(s);
    std::map<Symbol, std::vector<int> >::const_iterator it = mSongIDsInContent.find(c);
    if(it!=mSongIDsInContent.end())
        return it->second.size();
    else return 0;
}

void SongMgr::SetState(SongMgrState state){
    
}

void SongMgr::OnCacheMountResult(int){

}

void SongMgr::OnCacheWriteResult(int){

}

void SongMgr::OnCacheUnmountResult(int){

}

void SongMgr::CacheSongData(DataArray *, DataLoader *, ContentLocT, Symbol){

}

void SongMgr::ContentLoaded(Loader *, ContentLocT, Symbol){

}
