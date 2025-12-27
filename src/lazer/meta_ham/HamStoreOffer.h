#pragma once
#include "meta/SongMgr.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class HamStoreOffer : public StoreOffer {
public:
    virtual ~HamStoreOffer();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool Cmp(StoreOffer const &, Symbol) const;

    HamStoreOffer(DataArray *, SongMgr *);
    int Difficulty() const;

protected:
    String mAlbumArtPath; // 0x60
    String mPreviewPath; // 0x68
};
