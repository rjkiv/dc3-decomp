#pragma once

#include "SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "stl/_vector.h"
#include "types.h"
#include "utl/Symbol.h"


class StorePurchaseable : public Hmx::Object{
    public:
        StorePurchaseable();
        bool Exists() const;
        static unsigned long long OfferStringToID(char const *);
        char const * CostStr() const;

        bool unk2c;
        bool unk2d;
        u64 songID; //0x30
        int unk38;
};


class StoreOffer : public StorePurchaseable{
    public:
        Symbol OfferType() const;
        bool HasData(Symbol) const;
        Symbol FirstChar(Symbol, bool) const;
        Symbol PackFirstLetter() const;
        char const * OfferName() const;
        char const * ArtistName() const;
        char const * AlbumName() const;
        char const * Description() const;
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
        virtual DataNode Handle(DataArray *, bool);
        virtual ~StoreOffer();
        StoreOffer(DataArray *, SongMgr *);

        DataArray *arr; //0x40
        DateTime date; //0x44
        SongMgr *mSongMgr; //0x4c
        std::vector<int> unk50; //0x50, 0x54, 0x58
        
};
