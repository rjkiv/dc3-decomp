#include "meta/StoreOffer.h"
#include "SongMgr.h"
#include "macros.h"
#include "meta/Sorting.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "types.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <cstring>
#include <stdlib.h>

StorePurchaseable::StorePurchaseable() : 
    unk2c(0), unk2d(0), unk38(0), songID(0){}

bool StorePurchaseable::Exists() const{
    return (songID != 0) ? true : false;
}

unsigned long long StorePurchaseable::OfferStringToID(char const *s){
    return _strtoui64(s, nullptr, 16);
}

char const * StorePurchaseable::CostStr() const{
    return MakeString("%i -", unk38);
}



StoreOffer::StoreOffer(DataArray *a, SongMgr *mgr) : arr(a), mSongMgr(mgr), date(){
    DataNode* node;
    DataArray* focus;
    static Symbol id("id");
    static Symbol release_date("release_date");
    focus = a->FindArray(release_date,false);
    

    if(a->FindData(id,unk38,true)){
       songID = _strtoui64(id.Str(),nullptr,16);
    }

    focus = a->FindArray(release_date, false);
    if(focus!=nullptr){
        date = DateTime(focus->Int(3), focus->Int(2), focus->Int(1), 0,0,0);
    }

    static Symbol song_ids("song_ids");
    focus = a->FindArray(song_ids,false);

    if(focus!=nullptr && focus->Size() > 1){
        for(int i=1;i < focus->Size();i++){
            node = &focus->Node(i);
            unk50.push_back(date.GetDateFormatting());
        }
    }
    
    static Symbol avatar("avatar");
    if(unk50.empty() && OfferType().Str()!=avatar.Str()){
        MILO_NOTIFY("%s does not have song_ids", OfferName());
    }
    a->AddRef();
}

Symbol StoreOffer::OfferType() const{
    Symbol type("type");
    return nullptr;
}

StoreOffer::~StoreOffer(){arr->Release();}

bool StoreOffer::HasData(Symbol s) const{
    DataArray *array = arr->FindArray(s,false);
    return (array != nullptr);
}

Symbol StoreOffer::FirstChar(Symbol s, bool b) const{
    return nullptr;
 
}

Symbol StoreOffer::PackFirstLetter() const{
    return nullptr;
}

char const * StoreOffer::OfferName() const{
    return nullptr;
}

char const * StoreOffer::ArtistName() const{
    return nullptr;
}
char const * StoreOffer::AlbumName() const{
    return nullptr;
}
char const * StoreOffer::Description() const{
    return nullptr;
}

bool StoreOffer::IsNewRelease() const{
    return true;
}

bool StoreOffer::IsTest() const{
    return true;
}

int StoreOffer::NumSongs() const{
    return 0;
}

int StoreOffer::Song(int) const{
    return 0;
}

bool StoreOffer::ValidTitle() const{
    return true;
}

bool StoreOffer::InLibrary() const{
    return true;
}

bool StoreOffer::PartiallyInLibrary() const{
    return true;
}

int StoreOffer::GetSingleSongID() const{
    return 0;
}

DataNode StoreOffer::GetData(DataArray const *a, bool) const{
    return NULL_OBJ;
}

bool StoreOffer::HasSong(StoreOffer const *) const{
    return true;
}

DataNode StoreOffer::OnGetData(DataArray *){
    return NULL_OBJ;
}
