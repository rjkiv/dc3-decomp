#include "meta_ham/HamStoreProvider.h"
#include "HamStoreFilterProvider.h"
#include "macros.h"
#include "meta/StoreOffer.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamStorePanel.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/SongSortBySong.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_algo.h"
#include "stl/_pair.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/trie.h"

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
    : unk30(offers), unk34(filters), unk5c(0), mFilteredOffers(0), unkac(rows),
      mCartCheckout(nullptr) {
    unk74 = new HamStoreFilterProvider(unk34);
}

HamStoreProvider::~HamStoreProvider() {
    FOREACH (it, unk38) {
        RELEASE(it->second);
    }
    unk38.clear();
    mFilteredOffers = nullptr;
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

void HamStoreProvider::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT_RANGE(data, 0, mFilteredOffers->size(), 0x118);
    StoreOffer *offer = (*mFilteredOffers)[data];
    if (offer) {
        static Symbol store_checkout("store_checkout");
        Symbol storeData = offer->StoreOfferData()->Sym(0);
        if (storeData == store_checkout) {
            if (slot->Matches("song")) {
                label->SetTextToken(store_checkout);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("song")) {
            static Symbol by_artist("by_artist");
            static Symbol song("song");
            if (CurrentSort() == by_artist && offer->OfferType() == song) {
                static_cast<AppLabel *>(label)->SetStoreOfferArtist(offer);
            } else {
                static_cast<AppLabel *>(label)->SetStoreOfferName(offer);
            }
        } else if (slot->Matches("purchased")) {
            if (ShowBrowserPurchased(offer)) {
                label->SetTextToken(Symbol("store_purchased"));
            } else if (offer->InLibrary()) {
                label->SetTextToken(Symbol("store_in_library"));
            } else if (!offer->IsAvailable()) {
                label->SetTextToken(Symbol("store_unavailable"));
            } else {
                goto lol;
            }
        } else if (slot->Matches("cost")) {
            {
                String temp;
                if (!ShowBrowserPurchased(offer) && !offer->InLibrary()
                    && offer->IsAvailable()) {
                    static_cast<AppLabel *>(label)->SetStoreOfferCost(offer);
                    return;
                }
            }
            label->SetTextToken(gNullStr);
        } else if (slot->Matches("new") && offer->IsNewRelease()) {
            static Symbol new_content("new_content");
            label->SetTextToken(new_content);
        } else {
        lol:
            label->SetTextToken(gNullStr);
        }
    } else
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
    switch (i) {
    case 0:
        unkb0.push_back(offer);
        break;
    case 1:
        unkb0.remove(offer);
        break;
    case 2:
        unkb0.clear();
        break;
    }
    RefreshFilteredCartOffers();
}

void HamStoreProvider::SetPackList(StoreOffer const *offer) {
    static Symbol pack("pack");
    if (offer->OfferType() == pack) {
        static Symbol songs("songs");
        unk78.mSongs = offer->GetData(DataArrayPtr(songs), false).Array();
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
        if (!mSorts[mSortIndex].Null()) {
            std::sort(
                mFilteredOffers->begin(),
                mFilteredOffers->end(),
                SortCmp(mSorts[mSortIndex])
            );
        }
    }
}

std::list<StoreOffer *> *HamStoreProvider::GetOffersInCart() { return &unkb0; }

bool HamStoreProvider::ShowBrowserPurchased(const StoreOffer *offer) const {
    static Symbol song("song");
    static Symbol pack("pack");
    if (offer->IsPurchased()) {
        return true;
    } else {
        if (offer->OfferType() == song) {
            const StoreOffer *currOffer = FindPack(offer);
            if (currOffer && currOffer->HasSong(offer) && currOffer->IsPurchased()) {
                return true;
            }
        } else if (offer->OfferType() == pack) {
            for (int i = 0; i < offer->NumSongs(); i++) {
                const StoreOffer *currOffer = FindSong(offer->Song(i));
                if (!currOffer || !currOffer->IsPurchased()) {
                    return false;
                }
            }
            return offer->NumSongs() != 0;
        }
    }
    return false;
}

void HamStoreProvider::SetFilter(StoreOffer const *pack) {
    MILO_ASSERT(pack->OfferType()=="pack", 0xb0);
    unk50.clear();
    unk50.push_back((StoreOffer *)pack);
    for (int i = 0; i < pack->NumSongs(); i++) {
        const StoreOffer *offer = FindSong(pack->Song(i));
        if (offer && (offer->IsAvailable() || TheNetCacheMgr->IsDebug())) {
            unk50.push_back((StoreOffer *)offer);
        }
    }
    mFilteredOffers = &unk50;
    mSorts.clear();
    mSortIndex = 0;
}

void HamStoreProvider::PopulateOffersInCart() {
    HamStorePanel *storePanel = dynamic_cast<HamStorePanel *>(TheHamUI.FocusPanel());
    MILO_ASSERT(storePanel, 0x206);
    unkb0.clear();
    FOREACH_PTR (it_cartRow, unkac) {
        CartRow &row = *it_cartRow;
        FOREACH_PTR (it_storeOffer, unk30) {
            StoreOffer *offer = *it_storeOffer;
            if (offer->IsAvailable() || TheNetCacheMgr->IsDebug()) {
                static Symbol song("song");
                if (offer->OfferType() == song && offer->GetSingleSongID() == row.unk0) {
                    if (offer->IsPurchased()) {
                        storePanel->RemoveDLCFromCart(offer->GetSingleSongID());
                    } else {
                        unkb0.push_back(offer);
                    }
                    break;
                }
            }
        }
    }
    RefreshFilteredCartOffers();
}

void HamStoreProvider::OnNextSort() {
    MILO_ASSERT(AllowSortToggle(), 0xe8);
    mSortIndex = (mSortIndex + 1) % mSorts.size();
    ApplySort();
}

void HamStoreProvider::SetFilter(HamStoreFilter const *filter) {
    unk5c = (HamStoreFilter *)filter;
    unk50.clear();
    std::map<Symbol, std::vector<StoreOffer *> *>::iterator it;
    if (unk5c && (it = unk38.find(unk5c->unk0), it != unk38.end())) {
        mFilteredOffers = it->second;
        mSorts = unk5c->unkc;
    } else {
        unk5c = nullptr;
        mFilteredOffers = unk30;
        mSorts.clear();
    }
    mSortIndex = 0;
    ApplySort();
}

bool HamStoreProvider::IsOfferInCart(StoreOffer *offer) {
    FOREACH (it, unkb0) {
        if (offer == *it) {
            return true;
        }
    }
    return false;
}

void HamStoreProvider::RefreshFilteredCartOffers() {
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    auto it = unk38.find(store_filter_shopping_cart);
    if (it != unk38.end()) {
        RELEASE(it->second);
        unk38.erase(it);
    }

    if (unkb0.size() != 0) {
        if (!mCartCheckout) {
            static Symbol store_checkout("store_checkout");
            FOREACH_REVERSE_PTR(it, unk30) {
                Symbol data = (*it)->StoreOfferData()->Sym(0);
                if (data == store_checkout) {
                    mCartCheckout = *it;
                    break;
                }
            }
        }
        MILO_ASSERT(mCartCheckout, 0x242);
        std::vector<StoreOffer *> *offers = new std::vector<StoreOffer *>();
        offers->push_back(mCartCheckout);
        FOREACH (it, unkb0) {
            offers->push_back(*it);
        }
        unk38.insert(
            std::pair<Symbol, std::vector<StoreOffer *> *>(
                store_filter_shopping_cart, offers
            )
        );
    }
}

void HamStoreProvider::Refresh() {
    FOREACH (it, unk38) {
        RELEASE(it->second);
    }
    unk38.clear();
    mFilteredOffers = nullptr;

    FOREACH_PTR (it, unk30) {
        StoreOffer *offer = *it;
        if (offer->IsAvailable() || TheNetCacheMgr->IsDebug()) {
            DataArray *filters =
                offer->GetData(DataArrayPtr(Symbol("filters")), true).Array(0);
            for (int i = 1; i < filters->Size(); i++) {
                Symbol filterSym = filters->Sym(i);
                auto offerIt = unk38.find(filterSym);
                if (offerIt == unk38.end()) {
                    std::vector<StoreOffer *> *offers = new std::vector<StoreOffer *>();
                    offers->push_back(offer);
                    unk38.insert(
                        std::pair<Symbol, std::vector<StoreOffer *> *>(filterSym, offers)
                    );
                } else {
                    offerIt->second->push_back(offer);
                }
            }
        }
    }

    PopulateOffersInCart();
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    static Symbol store_filter_song_import_offers("store_filter_song_import_offers");
    for (auto it = unk34->begin(); it != unk34->end();) {
        auto filterIt = unk38.find((*it)->unk0);
        if ((filterIt == unk38.end() || filterIt->second->size() == 0)
            && (*it)->unk0 != store_filter_shopping_cart
            && (*it)->unk0 != store_filter_song_import_offers) {
            it = unk34->erase(it);
        } else {
            ++it;
        }
    }
    SetFilter(unk5c);
}

BEGIN_HANDLERS(HamStoreProvider)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_EXPR(get_offer, OnGetOffer(_msg->Int(2)))
    HANDLE_ACTION(set_pack, SetPackList(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(get_pack_provider, &unk78)
    HANDLE_EXPR(find_pack, (Hmx::Object *)FindPack(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(show_browser_purchased, ShowBrowserPurchased(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(show_unavailable, TheNetCacheMgr->IsDebug())
    HANDLE_EXPR(is_partially_purchased, IsPartiallyPurchased(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(allow_sort_toggle, mSorts.size() > 1)
    HANDLE_EXPR(get_current_sort_time, CurrentSort())
    HANDLE_ACTION(next_sort, OnNextSort())
    HANDLE_EXPR(is_offer_in_cart, IsOfferInCart(_msg->Obj<StoreOffer>(2)))
    HANDLE_EXPR(find_song, (Hmx::Object *)FindSong(_msg->Int(2)))
    HANDLE_EXPR(get_offer_index, OnGetOfferIndex(_msg->Obj<StoreOffer>(2)))
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion HamStoreProvider
