#include "meta_ham/HamStoreProvider.h"
#include "HamStoreFilterProvider.h"
#include "macros.h"
#include "meta/StoreOffer.h"
#include "meta_ham/AppLabel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_algo.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region PackSongListProvider

PackSongListProvider::PackSongListProvider() : mSongs() {}

void PackSongListProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(mSongs, 0x23);
    MILO_ASSERT_RANGE(data, 0, mSongs->Size(), 0x24);
    if (uiListLabel->Matches("song")) {
        static Symbol name("name");
        DataArray *array = mSongs->Node(data).Array(mSongs);
        static_cast<AppLabel *>(uiLabel)->SetPackSongName(array);
    } else {
        uiLabel->SetTextToken(gNullStr);
    }
}

int PackSongListProvider::NumData() const {
    if (mSongs)
        return mSongs->Size();
    return 0;
}

Symbol PackSongListProvider::DataSymbol(int idx) {
    MILO_ASSERT(mSongs, 0x3b);
    MILO_ASSERT_RANGE(idx, 0, mSongs->Size(), 0x3c);
    return mSongs->Node(idx).Array(mSongs)->Sym(0);
}

#pragma endregion PackSongListProvider
#pragma region HamStoreProvider

HamStoreProvider::HamStoreProvider(
    std::vector<StoreOffer *> *offers,
    std::vector<HamStoreFilter *> *filters,
    std::vector<CartRow> *rows
)
    : unk30(offers), unk34(filters), unk5c(0), mFilteredOffers(0), unkac(rows), unkb8(0) {
    unk74 = new HamStoreFilterProvider(unk34);
}

HamStoreProvider::~HamStoreProvider() {
    unk38.clear();
    RELEASE(unk74);
}

int HamStoreProvider::NumOffersInCart() {
    int offers = 0;
    FOREACH (it, unkb0) {
        offers++;
    }
    return offers;
}

int HamStoreProvider::NumData() const { return mFilteredOffers->size(); }

Symbol HamStoreProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mFilteredOffers->size(), 0x166);
    return (*mFilteredOffers)[idx]->StoreOfferData()->Sym(0);
}

void HamStoreProvider::Text(
    int, int data, UIListLabel *slot, UILabel *label
) const {
    MILO_ASSERT_RANGE(data, 0, mFilteredOffers->size(), 0x118);
    StoreOffer *offer = (*mFilteredOffers)[data];
    if (!offer) {
        label->SetTextToken(gNullStr);
        return;
    }
    static Symbol store_checkout("store_checkout");
    if (offer->StoreOfferData()->Sym(0) == store_checkout) {
        if (slot->Matches("song")) {
            label->SetTextToken(store_checkout);
            return;
        }
    } else {
        if (slot->Matches("song")) {
            static Symbol by_artist("by_artist");
            static Symbol song("song");
            if (CurrentSort() == by_artist && offer->OfferType() == song) {
                static_cast<AppLabel *>(label)->SetStoreOfferArtist(offer);
            } else {
                static_cast<AppLabel *>(label)->SetStoreOfferName(offer);
            }
            return;
        } else if (slot->Matches("purchased")) {
            if (ShowBrowserPurchased(offer)) {
                label->SetTextToken(Symbol("store_purchased"));
                return;
            } else if (offer->InLibrary()) {
                label->SetTextToken(Symbol("store_in_library"));
                return;
            } else if (!offer->IsAvailable()) {
                label->SetTextToken(Symbol("store_unavailable"));
                return;
            }
        } else if (slot->Matches("cost")) {
            String temp;
            if (!ShowBrowserPurchased(offer) && !offer->InLibrary() && offer->IsAvailable()) {
                static_cast<AppLabel *>(label)->SetStoreOfferCost(offer);
            }
            return;
        } else if (slot->Matches("new")) {
            if (offer->IsNewRelease()) {
                static Symbol new_content("new_content");
                label->SetTextToken(new_content);
                return;
            }
        }
    }
    label->SetTextToken(gNullStr);
}

int HamStoreProvider::OnGetOfferIndex(StoreOffer *offer) {
    if (offer) {
        for (int i = 0; i < mFilteredOffers->size(); i++) {
            if ((*mFilteredOffers)[i] == offer)
                return i;
        }
    }
    return -1;
}

StoreOffer *HamStoreProvider::OnGetOffer(int idx) {
    MILO_ASSERT_RANGE(idx, 0, mFilteredOffers->size(), 0x1da);
    return (*mFilteredOffers)[idx];
}

StoreOffer const *HamStoreProvider::FindPack(StoreOffer const *song) const {
    MILO_ASSERT(song->OfferType() == "song", 0x18e);
    static Symbol pack("pack");
    FOREACH_PTR (it, unk30) {
        if ((*it)->OfferType() == pack && (*it)->HasSong(song))
            return *it;
    }
    return nullptr;
}

StoreOffer const *HamStoreProvider::FindSong(int id) const {
    static Symbol song("song");
    FOREACH_PTR (it, unk30) {
        StoreOffer *so = *it;
        if (so->OfferType() == song && so->GetSingleSongID() == id)
            return so;
    }
    return nullptr;
}

Symbol HamStoreProvider::CurrentSort() const {
    if (mSorts.size() > 1) {
        return mSorts[mSortIndex];
    }
    return gNullStr;
}

void HamStoreProvider::UpdateOffersInCart(StoreOffer *offer, int i) {
    if (i == 0) {
        unkb0.push_back(offer);
    } else if (i == 1) {
        unkb0.remove(offer);
    } else if (i < 3) {
        unkb0.clear();
    }
    RefreshFilteredCartOffers();
}

void HamStoreProvider::SetPackList(StoreOffer const *offer) {
    static Symbol pack("pack");
    if (offer->OfferType() == pack) {
        static Symbol songs("songs");
        DataArrayPtr ptr = DataArrayPtr(songs);
        unk78.mSongs = offer->GetData(ptr, false).Array(0);
        unk78.mSongs->Release();
    } else {
        unk78.mSongs = 0;
    }
}

bool HamStoreProvider::IsPartiallyPurchased(StoreOffer const *offer) const {
    static Symbol song("song");
    static Symbol pack("pack");
    if (ShowBrowserPurchased(offer)) {
        return true;
    } else {
        if (offer->OfferType() == pack) {
            for (int i = 0; i < offer->NumSongs(); i++) {
                const StoreOffer *song = FindSong(offer->Song(i));
                if (song && const_cast<StoreOffer *>(song)->IsPurchased()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void HamStoreProvider::ApplySort() {
    if (!mSorts.empty()) {
        MILO_ASSERT_RANGE(mSortIndex, 0, mSorts.size(), 0xf1);
        if (mSorts[mSortIndex].Str() != gNullStr) {
            std::sort(mFilteredOffers->begin(), mFilteredOffers->end(), SortCmp());
        }
    }
}

BEGIN_HANDLERS(HamStoreProvider)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_EXPR(get_offer, OnGetOffer(_msg->Int(2)))
    HANDLE_ACTION(set_pack, SetPackList(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(get_pack_provider, &GetPackProvider()) // recheck
    HANDLE_ACTION(find_pack, FindPack(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(show_browser_purchased, ShowBrowserPurchased(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(show_unavailable, TheNetCacheMgr->IsDebug())
    HANDLE_EXPR(is_partially_purchased, IsPartiallyPurchased(_msg->Obj<StoreOffer>(2)))
    // HANDLE_EXPR(allow_sort_toggle, expr)
    HANDLE_EXPR(get_current_sort_time, CurrentSort())
    HANDLE_ACTION(next_sort, OnNextSort())
    // is offer in cart
    HANDLE_ACTION(find_song, FindSong(_msg->Int(2)))
    HANDLE_EXPR(get_offer_index, OnGetOfferIndex(_msg->Obj<StoreOffer>(2)))
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion HamStoreProvider
