#include "meta_ham/HamStorePanel.h"
#include "HamStoreOffer.h"
#include "meta/SongMgr.h"
#include "meta/StoreOffer.h"
#include "meta/StorePanel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamStoreProvider.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/HamStoreCartJobs.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/System.h"
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
