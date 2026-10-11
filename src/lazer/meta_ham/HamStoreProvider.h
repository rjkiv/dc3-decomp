#pragma once
#include "meta/StoreOffer.h"
#include "meta_ham/HamStoreFilterProvider.h"
#include "net_ham/HamStoreCartJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/NetCacheMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

class PackSongListProvider : public UIListProvider, public Hmx::Object {
public:
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int);
    virtual int NumData() const;

    PackSongListProvider();

    DataArray *mSongs; // 0x30
};

class HamStoreProvider : public UIListProvider, public Hmx::Object {
public:
    virtual ~HamStoreProvider();
    virtual DataNode Handle(DataArray *, bool);

    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const;

    HamStoreProvider(
        std::vector<StoreOffer *> *,
        std::vector<HamStoreFilter *> *,
        std::vector<CartRow> *
    );
    StoreOffer const *FindSong(int) const;
    StoreOffer const *FindPack(StoreOffer const *) const;
    bool ShowBrowserPurchased(StoreOffer const *) const;
    bool IsPartiallyPurchased(StoreOffer const *) const;
    void SetPackList(StoreOffer const *);
    Symbol CurrentSort() const;
    int NumOffersInCart();
    void UpdateOffersInCart(StoreOffer *, int);
    void SetFilter(StoreOffer const *);
    void SetFilter(HamStoreFilter const *);
    void OnNextSort();
    void Refresh();
    std::list<StoreOffer *> *GetOffersInCart();
    bool IsOfferInCart(StoreOffer *);
    bool AllowSortToggle() { return mSorts.size() > 1; }
    bool IsDebug();

    HamStoreFilterProvider *GetFilterProvider() { return mFilterProvider; }

private:
    StoreOffer *OnGetOffer(int);
    int OnGetOfferIndex(StoreOffer *);
    void RefreshFilteredCartOffers();
    void PopulateOffersInCart();
    void ApplySort();

    std::vector<StoreOffer *> *mOffers; // 0x30
    std::vector<HamStoreFilter *> *mFilters; // 0x34
    std::map<Symbol, std::vector<StoreOffer *> *> mFilterToOffersMap; // 0x38
    std::vector<StoreOffer *> mPackSongs; // 0x50
    const HamStoreFilter *mFilter; // 0x5c
    int mSortIndex; // 0x60
    std::vector<StoreOffer *> *mFilteredOffers; // 0x64
    std::vector<Symbol> mSorts; // 0x68
    HamStoreFilterProvider *mFilterProvider; // 0x47
    PackSongListProvider mPackSongList; // 0x78
    std::vector<CartRow> *mCartData; // 0xac
    std::list<StoreOffer *> mOffersInCart; // 0xb0
    StoreOffer *mCartCheckout; // 0xb8
};
