#include "meta_ham/HamStorePanel.h"
#include "HamStoreOffer.h"
#include "meta/SongMgr.h"
#include "meta/StoreOffer.h"
#include "meta/StorePanel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamStoreProvider.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/HamStoreCartJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/User.h"
#include "utl/Symbol.h"

HamStorePanel::HamStorePanel()
    : unka0(), unka4(), mOfferProvider(), unkb8(), unkc0(false), unk128(), unk154(false),
      unk155(true), unk156(false), unk157(false), unk158(false), unk159(false), unk184(),
      unk188() {
    for (int i = 7; i != 0; i--) {
        unk138[i] = 0;
    }
    DataArray *sysConfig = SystemConfig("store");
    DataArray *specialOfferArray = sysConfig->FindArray("special_offers", false);
    if (specialOfferArray) {
    }
    TheContentMgr.RegisterCallback(this, false);
}

HamStorePanel::~HamStorePanel() { TheContentMgr.UnregisterCallback(this, false); }

BEGIN_PROPSYNCS(HamStorePanel)
    SYNC_SUPERCLASS(StorePanel)
END_PROPSYNCS

void HamStorePanel::Load() {
    StorePanel::Load();
    MILO_ASSERT(!mOfferProvider, 0xd3);
    mOfferProvider = new HamStoreProvider(&unk38, &unkac, &unk12c);
    unkc0 = false;
    unkc4.Restart();
    RefreshSpecialOfferStatus();
}

bool HamStorePanel::Exiting() const {
    if (unk188 != 0)
        return true;
    return StorePanel::Exiting();
}

bool HamStorePanel::IsSongInLibrary(const int &id) const {
    return TheSongMgr.HasSong(id);
}

Profile *HamStorePanel::StoreProfile() const {
    return TheProfileMgr.GetActiveProfile(true);
}

StoreOffer *HamStorePanel::MakeNewOffer(DataArray *d) {
    return new HamStoreOffer(d, &TheSongMgr);
}

void HamStorePanel::DisableCart() {
    if (unk157) {
        MILO_FAIL("Can\'t disable the cart after it is loaded");
    }
    unk155 = false;
    unk128 = 0;
}

void HamStorePanel::RemoveDLCFromCart(int id) {
    unk15c.push_back(id);
    if (!unk158) {
        unk158 = true;
        RemoveNextDLCFromCart();
    }
}

void HamStorePanel::AddDLCToCart(int id) {
    unk164.push_back(id);
    if (!unk159) {
        unk159 = true;
        AddNextDLCToCart();
    }
}

void HamStorePanel::RemoveOfferFromCart(StoreOffer *offer) {
    mOfferProvider->UpdateOffersInCart(offer, 1);
    RemoveDLCFromCart(offer->GetSingleSongID());
}

bool HamStorePanel::IsCurrFilterCart(int id) {
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    return unkac[id]->unk0 == store_filter_shopping_cart;
}

void HamStorePanel::GetCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x23d);
    unk138[3] = new GetCartJob(this, profile);
    TheRockCentral.ManageJob(unk138[3]);
}

void HamStorePanel::LockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x246);
    unk156 = true;
    unk128 = 0;
}

void HamStorePanel::StoreUserProfileSwappedToUser(LocalUser *) {
    RefreshSpecialOfferStatus();
}

void HamStorePanel::ReadLockData() {
    ((LockCartJob *)unk138[4])->GetLockData(unk128);
    unk138[4] = nullptr;
    unkf8.Restart();
}

BEGIN_HANDLERS(HamStorePanel)
    HANDLE_EXPR(get_motd, unkb8)
    HANDLE_ACTION(set_filter, mOfferProvider->SetFilter(unkac[_msg->Int(2)]))
    HANDLE_ACTION(
        set_filter_pack_singles, mOfferProvider->SetFilter(_msg->Obj<StoreOffer>(2))
    )
    // HANDLE_EXPR(offer_provider, )
    // filter_provider
    // reset_cancel_timer
    // allow_cancel
    // is_cart_enabled
    HANDLE_ACTION(disable_cart, DisableCart())
    HANDLE_ACTION(get_cart, GetCart())
    HANDLE_ACTION(add_offer_to_cart, AddOfferToCart(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(remove_offer_from_cart, RemoveOfferFromCart(_msg->Obj<StoreOffer>(2)))
    // HANDLE_ACTION(cart_checkout, MultipleItemsCheckout())
    HANDLE_ACTION(lock_cart, LockCart())
    HANDLE_ACTION(unlock_cart, UnlockCart())
    HANDLE_EXPR(is_curr_filter_cart, IsCurrFilterCart(_msg->Int(2)))
    HANDLE_EXPR(is_cart_empty, mOfferProvider->NumOffersInCart() == 0) // recheck
    HANDLE_EXPR(is_cart_full, mOfferProvider->NumOffersInCart() < 10) // recheck i made up
                                                                      // a number
    HANDLE_ACTION(empty_cart, EmptyCart())
    HANDLE_ACTION(set_filter_to_cart, SetFilterToCart())
    HANDLE_ACTION(set_filter_to_songs, SetFilterToSongs())
    HANDLE_ACTION(refresh_special_offers, RefreshSpecialOfferStatus())
    HANDLE_EXPR(check_owned, IsSpecialOfferOwned(_msg->ForceSym(2)))
    HANDLE_EXPR(buy_special, BuySpecialOffer(_msg->ForceSym(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_ACTION(buy_dc1_import, BuySpecialOffer("dc1_import"))
    HANDLE_ACTION(buy_dc2_import, BuySpecialOffer("dc2_import"))
    HANDLE_ACTION(buy_dc2_pop, BuySpecialOffer("dc2_pop"))
    HANDLE_ACTION(buy_dc2_gond, BuySpecialOffer("dc2_gond"))
    HANDLE_SUPERCLASS(StorePanel)
END_HANDLERS
