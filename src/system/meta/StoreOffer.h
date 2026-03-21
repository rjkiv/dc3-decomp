#pragma once
#include "SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "stl/_vector.h"
#include "types.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"

class StorePurchaseable : public Hmx::Object {
public:
    StorePurchaseable();
    bool Exists() const;
    static unsigned long long OfferStringToID(char const *);
    char const *CostStr() const;
    bool IsAvailable() const { return isAvailable; }
    bool IsPurchased() const { return isPurchased; }
    unsigned long long SongID() const { return songID; }

    bool isAvailable; // 0x2c
    bool isPurchased; // 0x2d
    unsigned long long songID; // 0x30
    int cost; // 0x38
};

class StoreOffer : public StorePurchaseable {
public:
    // Hmx::Object
    virtual ~StoreOffer();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool Cmp(StoreOffer const &, Symbol) const = 0;

    bool HasData(Symbol) const;
    DateTime const &ReleaseDate() const;
    Symbol FirstChar(Symbol, bool) const;
    Symbol PackFirstLetter() const;
    char const *OfferName() const;
    char const *ArtistName() const;
    char const *AlbumName() const;
    char const *Description() const;
    bool IsNewRelease() const;
    bool IsTest() const;
    int NumSongs() const;
    int Song(int) const;
    bool ValidTitle() const;
    bool InLibrary() const;
    bool PartiallyInLibrary() const;
    int GetSingleSongID() const;
    DataNode GetData(DataArray const *, bool) const;
    bool HasSong(StoreOffer const *) const;
    DataNode OnGetData(DataArray *);
    StoreOffer(DataArray *, SongMgr *);

    DataArray *StoreOfferData() const { return mStoreOfferData; }
    Symbol OfferType() const {
        static Symbol type("type");
        return mStoreOfferData->FindSym(type);
    }

protected:
    DataArray *mStoreOfferData; // 0x40
    DateTime date; // 0x44
    SongMgr *mSongMgr; // 0x4c
    std::vector<int> mSongsInOffer; // 0x50
};

class SortCmp {
public:
    SortCmp(Symbol type) : mType(type) {}
    bool operator()(const StoreOffer *offer1, const StoreOffer *offer2) const {
        return offer1->Cmp(*offer2, mType);
    }

    Symbol mType;
};
