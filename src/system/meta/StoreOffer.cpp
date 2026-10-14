#include "meta/StoreOffer.h"
#include "SongMgr.h"
#include "macros.h"
#include "meta/Sorting.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
#include "types.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include <cstring>
#include <stdlib.h>

StorePurchaseable::StorePurchaseable()
    : mAvailable(0), mPurchased(0), mPointsPrice(0), mOfferID(0) {}

bool StorePurchaseable::Exists() const { return (mOfferID != 0) ? true : false; }

unsigned long long StorePurchaseable::OfferStringToID(char const *s) {
    return _strtoui64(s, nullptr, 16);
}

char const *StorePurchaseable::CostStr() const {
    return MakeString("%i -", mPointsPrice);
}

StoreOffer::StoreOffer(DataArray *a, SongMgr *mgr) : mData(a), mSongMgr(mgr) {
    static Symbol id("id");
    static Symbol release_date("release_date");

    char const *c;
    if (mData->FindData(id, c, false)) {
        SetOfferID(_strtoui64(c, nullptr, 16));
    }

    DataArray *dateArray = mData->FindArray(release_date, false);
    if (dateArray) {
        DateTime date =
            DateTime(dateArray->Int(1), dateArray->Int(2), dateArray->Int(3), 0, 0, 0);
        mReleaseDate = date;
    }

    static Symbol song_ids("song_ids");
    DataArray *idArray = mData->FindArray(song_ids, false);
    if (idArray) {
        for (int i = 1; i < idArray->Size(); i++) {
            mSongsInOffer.push_back(idArray->Int(i));
        }
    }

    static Symbol avatar("avatar");
    if (mSongsInOffer.empty() && OfferType() != avatar) {
        MILO_NOTIFY("%s does not have song_ids", OfferName());
    }
    mData->AddRef();
}

StoreOffer::~StoreOffer() { mData->Release(); }

bool StoreOffer::HasData(Symbol s) const {
    return (mData->FindArray(s, false) != nullptr);
}

DateTime const &StoreOffer::ReleaseDate() const { return mReleaseDate; }

Symbol StoreOffer::FirstChar(Symbol s, bool b) const {
    return FirstSortChar(mData->FindStr(s), b);
}

Symbol StoreOffer::PackFirstLetter() const {
    static Symbol pack("pack");
    static Symbol name("name");

    if (OfferType() == pack) {
        return FirstSortChar(mData->FindStr(name), 1);
    } else
        return gNullStr;
}

char const *StoreOffer::OfferName() const {
    static Symbol name("name");
    return mData->FindStr(name);
}

char const *StoreOffer::ArtistName() const {
    static Symbol artist("artist");
    return mData->FindStr(artist);
}
char const *StoreOffer::AlbumName() const {
    static Symbol album_name("album_name");
    return mData->FindStr(album_name);
}
char const *StoreOffer::Description() const {
    static Symbol description("description");
    return mData->FindStr(description);
}

bool StoreOffer::IsNewRelease() const {
    static Symbol new_release("new_release");
    DataArray *r = mData->FindArray(new_release, false);
    if (r != 0) {
        return r->Int(1) != 0;
    }
    return false;
}

bool StoreOffer::IsTest() const {
    static Symbol test("test");
    DataArray *t = mData->FindArray(test, false);
    if (t != nullptr) {
        return t->Int(1);
    }

    return false;
}

int StoreOffer::NumSongs() const {
    MILO_ASSERT(OfferType() == "pack" || OfferType() == "album", 0x100);
    return mSongsInOffer.size();
}

int StoreOffer::Song(int i) const {
    MILO_ASSERT(mSongsInOffer.size() > i, 0x106);
    return mSongsInOffer[i];
}

bool StoreOffer::ValidTitle() const {
    static Symbol titles("titles");
    DataArray *titleArray = mData->FindArray(titles, false);
    if (titleArray != nullptr) {
        for (int i = 1; i < titleArray->Size(); i++) {
            if (SystemTitles()->FindArray(titleArray->Sym(0), false)) {
                return true;
            }
        }
        return false;
    }
    return true;
}

bool StoreOffer::InLibrary() const {
    for (std::vector<int>::const_iterator it = mSongsInOffer.begin();
         it != mSongsInOffer.end();
         ++it) {
        int i = *it;
        if (mSongMgr == nullptr || !mSongMgr->HasSong(i))
            return false;
    }
    return mSongsInOffer.size() > 0;
}

bool StoreOffer::PartiallyInLibrary() const {
    for (std::vector<int>::const_iterator it = mSongsInOffer.begin();
         it != mSongsInOffer.end();
         ++it) {
        int i = *it;
        if (mSongMgr != nullptr && mSongMgr->HasSong(i))
            return true;
    }
    return false;
}

int StoreOffer::GetSingleSongID() const {
    if (mSongsInOffer.empty()) {
        MILO_NOTIFY("No SongIDs in an offer that expects to have one!");
        return 0;
    } else if (1 < mSongsInOffer.size()) {
        MILO_NOTIFY("More than one SongID in an offer that expects to just have one!");
    }
    return mSongsInOffer.front();
}

DataNode StoreOffer::GetData(DataArray const *data, bool b) const {
    DataArray *storeData = mData;
    for (int i = 0; i < data->Size(); i++) {
        storeData = storeData->FindArray(data->Sym(i));
    }
    return b ? storeData : storeData->Node(1);
}

bool StoreOffer::HasSong(StoreOffer const *o) const {
    MILO_ASSERT(OfferType() == "pack" || OfferType() == "album",0x10c);
    MILO_ASSERT(o->OfferType() == "song", 0x10d);
    for (int i = 0; i < NumSongs(); i++) {
        if (Song(i) == o->GetSingleSongID()) {
            return true;
        }
    }

    return false;
}

DataNode StoreOffer::OnGetData(DataArray *d) {
    DataArray *array = d->Array(2);
    int x;
    if (3 < d->Size()) {
        x = d->Int(3);
    } else {
        x = 0;
    }
    return GetData(array, x != 0);
}

BEGIN_HANDLERS(StoreOffer)
    HANDLE_EXPR(short_name, ShortName())
    HANDLE_EXPR(offer_type, OfferType())
    HANDLE_EXPR(offer_name, OfferName())
    HANDLE_EXPR(artist, ArtistName())
    HANDLE_EXPR(album_name, AlbumName())
    HANDLE_EXPR(description, Description())
    HANDLE_EXPR(is_new_release, IsNewRelease())
    HANDLE_EXPR(cost_str, MakeString("%i -", mPointsPrice))
    HANDLE_EXPR(in_library, InLibrary())
    HANDLE_EXPR(partially_in_library, PartiallyInLibrary())
    HANDLE_EXPR(is_available, mAvailable)
    HANDLE_EXPR(is_purchased, mPurchased)
    HANDLE_EXPR(is_test, IsTest())
    HANDLE(get_data, OnGetData)
    HANDLE_EXPR(has_data, HasData(_msg->Sym(2)))
    HANDLE_EXPR(first_char, FirstChar(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(pack_first_letter, PackFirstLetter())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
