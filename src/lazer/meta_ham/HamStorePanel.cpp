#include "meta_ham/HamStorePanel.h"
#include "HamStoreOffer.h"
#include "macros.h"
#include "meta/SongMgr.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "meta/StorePanel.h"
#include "meta/StorePurchaser.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamStoreFilterProvider.h"
#include "meta_ham/HamStoreProvider.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/UIEventMgr.h"
#include "net_ham/HamStoreCartJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/User.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/NetCacheMgr.h"
#include "utl/NetLoader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

HamStorePanel::HamStorePanel()
    : unka0(), mMetadata(), mOfferProvider(), unkb8(), unkc0(false), unk128(),
      unk154(false), unk155(true), unk156(false), unk157(false), unk158(false),
      unk159(false), unk184(-1), mXboxPurchaser() {
    for (int i = 0; i < 7; i++) {
        unk138[i] = 0;
    }
    DataArray *specialOfferArray =
        SystemConfig("store")->FindArray("special_offers", false);
    if (specialOfferArray) {
        int numOffers = specialOfferArray->Size() - 1;
        HamSpecialOffer tempOffer;
        unk16c.resize(numOffers, tempOffer);
        unk178.resize(numOffers, 0);

        for (int i = 0; i < numOffers; i++) {
            DataArray *offerArray = specialOfferArray->Array(i + 1);
            HamSpecialOffer &offer = unk16c[i];
            offer.unk4 = offerArray->Sym(0);
            offer.unk14 = false;
            offer.unk8 = StorePurchaseable::OfferStringToID(offerArray->Str(1));
            unk178[i] = offer.unk8;
            offer.unk10 = offerArray->ForceSym(2);
        }
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
    if (mXboxPurchaser != 0)
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
    unk138[4] = new LockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(unk138[4]);
}

void HamStorePanel::UnlockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x252);
    unk156 = false;
    unk138[5] = new UnlockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(unk138[5]);
}

void HamStorePanel::RelockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x25d);
    unk156 = true;
    unk138[6] = new LockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(unk138[6]);
    unkf8.Restart();
}

void HamStorePanel::EmptyCart() {
    mOfferProvider->UpdateOffersInCart(nullptr, 2);
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x26c);
    unk138[2] = new EmptyCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(unk138[2]);
}

void HamStorePanel::StoreUserProfileSwappedToUser(LocalUser *) {
    RefreshSpecialOfferStatus();
}

void HamStorePanel::ReadLockData() {
    ((LockCartJob *)unk138[4])->GetLockData(unk128);
    unk138[4] = nullptr;
    unkf8.Restart();
}

void HamStorePanel::ReadCartData() {
    GetCartJob *job = (GetCartJob *)unk138[3];
    unk12c.clear();
    job->GetRows(&unk12c);
    unk138[3] = nullptr;
    unk157 = true;
}

StoreOffer *HamStorePanel::FindOffer(Symbol offerName) const {
    FOREACH (it, unk38) {
        StoreOffer *offer = *it;
        Symbol s = offer->StoreOfferData()->Sym(0);
        if (s == offerName)
            return offer;
    }
    return nullptr;
}

void HamStorePanel::SetFilterToCart() {
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    for (int i = unkac.size() - 1; i >= 0; i--) {
        if (unkac[i]->unk0 == store_filter_shopping_cart) {
            mOfferProvider->SetFilter(unkac[i]);
            return;
        }
    }
}

int HamStorePanel::SetFilterToSongs() {
    static Symbol songs("songs");
    for (int i = unkac.size() - 1; i >= 0; i--) {
        if (unkac[i]->unk0 == songs) {
            mOfferProvider->SetFilter(unkac[i]);
            return i;
        }
    }
    return -1;
}

void HamStorePanel::AddNextDLCToCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x2da);
    unk138[0] =
        new AddDLCToCartJob(this, profile->GetOnlineID()->ToString(), unk164.front());
    TheRockCentral.ManageJob(unk138[0]);
}

void HamStorePanel::RemoveNextDLCFromCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x2c0);
    unk138[1] = new RemoveDLCFromCartJob(
        this, profile->GetOnlineID()->ToString(), unk15c.front()
    );
    TheRockCentral.ManageJob(unk138[1]);
}

void HamStorePanel::AddOfferToCart(StoreOffer *offer) {
    mOfferProvider->UpdateOffersInCart(offer, 0);
    AddDLCToCart(offer->GetSingleSongID());
}

char const *HamStorePanel::GetIndexFile() const {
    const char *str = "store_index_%s_%s.dtz";
    Symbol platSym = PlatformSymbol(TheLoadMgr.GetPlatform());
    Symbol sysLang = SystemLanguage();
    return MakeString(str, sysLang, platSym);
}

void HamStorePanel::ExitStore(StoreError err) const {
    static Symbol store_load_failed("store_load_failed");
    if (TheUIEventMgr->CurrentEvent() != store_load_failed) {
        static Message init("init", -1);
        init[0] = err;
        TheUIEventMgr->TriggerEvent(store_load_failed, init);
    }
}

void HamStorePanel::CreateCartUIs() {
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    static Symbol store_checkout("store_checkout");
    static Symbol type("type");
    static Symbol fake("fake");
    static Symbol name("name");
    static Symbol artist("artist");
    static Symbol album_name("album_name");
    static Symbol description("description");
    static Symbol art("art");
    static Symbol store_filter_song_import_offers("store_filter_song_import_offers");
    unkac.insert(unkac.begin(), 1, new HamStoreFilter(store_filter_shopping_cart));
    unkac.push_back(new HamStoreFilter(store_filter_song_import_offers));
}

bool HamStorePanel::IsSpecialOfferOwned(Symbol offer) const {
    FOREACH (it, unk16c) {
        if ((*it).unk4 == offer) {
            return (*it).unk14;
        }
    }
    Symbol s = offer; // bruh
    MILO_NOTIFY("Unknown offer %s", s);
    return false;
}

void HamStorePanel::ResetCancelTimer() {
    unkc0 = false;
    unkc4.Restart();
}

bool HamStorePanel::ContentTitleDiscovered(unsigned int ui, Symbol s) {
    static Symbol dc2_gond("dc2_gond");
    if (ui == 0x373307d2) {
        FOREACH (it, unk16c) {
            if (it->unk4 == dc2_gond) {
                it->unk10 = s;
                break;
            }
        }
        return false;
    }
    return true;
}

void HamStorePanel::ContentMounted(char const *c1, char const *c2) {
    FOREACH (it, unk16c) {
        if (it->unk10 == c1) {
            Symbol s = it->unk4;
            MILO_LOG("Store: special offer %s on local drive.\n", s);
            it->unk14 = true;
            return;
        }
    }
}

bool HamStorePanel::ContentDiscovered(Symbol s) {
    FOREACH (it, unk16c) {
        if (it->unk10 == s) {
            return false;
        }
    }
    return true;
}

bool HamStorePanel::BuySpecialOffer(Symbol offer) {
    if (mXboxPurchaser) {
        MILO_FAIL("There is a purchase in progress.");
    }
    FOREACH (it, unk16c) {
        if ((*it).unk4 == offer) {
            Profile *profile = StoreProfile();
            if (profile) {
                mXboxPurchaser =
                    new XboxPurchaser(profile->GetPadNum(), it->unk8, 0, 0, unk8c, 0);
                mXboxPurchaser->Initiate();
            }
            return true;
        }
    }
    Symbol s = offer;
    MILO_NOTIFY("Unknown offer %s", s);
    return false;
}

void HamStorePanel::Poll() {
    StorePanel::Poll();
    if (!unkc0 && unkc4.SplitMs() > 5000.0f) {
        unkc0 = true;
        if (TheHamUI.FocusPanel() == this) {
            TheHamUI.GetHelpBarPanel()->SyncToPanel(this);
        }
    }
    if (!mLoadOk) {
        return;
    }
    if (!TheNetCacheMgr->IsReady()) {
        return;
    }
    if (unk94 != 2) {
        return;
    }
    if (unka0) {
        unka0->PollLoading();
        if (unka0->IsLoaded()) {
            mMetadata = unka0->GetUnk4();
            MILO_ASSERT(mMetadata, 0xfe);
            mMetadata->AddRef();
            static Symbol motd("motd");
            mMetadata->FindData(motd, unkb8, false);
            DataArray *filterArray = mMetadata->FindArray("filters", true);
            DeleteAll(unkac);
            for (int i = 1; i < filterArray->Size(); i++) {
                unkac.push_back(new HamStoreFilter(filterArray->Array(i)));
            }
            DataArray *offerArray = mMetadata->FindArray("offers", true);
            PopulateOffers(offerArray, false);
            CreateCartUIs();
            unk154 = true;
        } else {
            if (!unka0->HasFailed()) {
                goto tag;
            }
            MILO_NOTIFY("Request for %s failed.", GetIndexFile());
            ExitError((StoreError)3);
        }
        RELEASE(unka0);
    } else {
        if (mMetadata == 0) {
            String indexFile = GetIndexFile();
            unka0 = new DataNetLoader(indexFile);
        } else if (unk154 && (unk157 || !unk155)) {
            unk154 = false;
            unk70 = true;
        }
    }
tag:
    if (unk156 && unk128 != 0) {
        if (unkf8.SplitMs() >= unk128) {
            RelockCart();
        }
    }
    if (mXboxPurchaser) {
        mXboxPurchaser->Poll();
        if (!mXboxPurchaser->IsPurchasing()) {
            bool purchaseMade = false;
            bool needsEnum = false;
            if (mXboxPurchaser->IsSuccess()) {
                purchaseMade = mXboxPurchaser->PurchaseMade();
                needsEnum = mXboxPurchaser->NeedsEnum();
                if (purchaseMade && needsEnum) {
                    RefreshSpecialOfferStatus();
                }
            }

            static Message special_finished("special_finished", 0, 0);
            special_finished[0] = purchaseMade;
            special_finished[1] = needsEnum;
            HandleType(special_finished);
            TheUI->Handle(special_finished, false);
            RELEASE(mXboxPurchaser);
        }
    }
}

void HamStorePanel::Unload() {
    RELEASE(unka0);
    RELEASE(mOfferProvider);
    if (mMetadata) {
        mMetadata->Release();
        mMetadata = nullptr;
    }
    DeleteAll(unkac);
    unkc4.Stop();
    unkc0 = false;
    unkf8.Stop();
    unk128 = 0;
    unk155 = true;
    unk154 = false;
    unk157 = false;
    unk158 = false;
    unk159 = false;
    if (unk156) {
        UnlockCart();
    }
    StorePanel::Unload();
}

void HamStorePanel::FinishSpecialOfferEnum(std::vector<bool> const &vec, bool b) {
    unk184 = -1;
    if (!b) {
        MILO_LOG("Store: failed to enum our special offers.\n");
    } else {
        for (int i = 0; i < unk16c.size(); i++) {
            if (!unk16c[i].unk14) {
                unk16c[i].unk14 = vec[i];
            }

            if (unk16c[i].unk14) {
                MILO_LOG("Store: special offer %s is owned\n", unk16c[i].unk4);
            }
        }
    }
    static Message refresh_complete("refresh_complete", 0);
    refresh_complete[0] = b;
    TheUI->Handle(refresh_complete, false);
}

BEGIN_HANDLERS(HamStorePanel)
    HANDLE_EXPR(get_motd, unkb8)
    HANDLE_ACTION(set_filter, mOfferProvider->SetFilter(unkac[_msg->Int(2)]))
    HANDLE_ACTION(
        set_filter_pack_singles, mOfferProvider->SetFilter(_msg->Obj<StoreOffer>(2))
    )
    HANDLE_EXPR(offer_provider, (Hmx::Object *)mOfferProvider)
    HANDLE_EXPR(filter_provider, (Hmx::Object *)mOfferProvider->GetFilterProvider())
    HANDLE_ACTION(reset_cancel_timer, ResetCancelTimer())
    HANDLE_EXPR(allow_cancel, unkc0)
    HANDLE_EXPR(is_cart_enabled, unk155)
    HANDLE_ACTION(disable_cart, DisableCart())
    HANDLE_ACTION(get_cart, GetCart())
    HANDLE_ACTION(add_offer_to_cart, AddOfferToCart(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(remove_offer_from_cart, RemoveOfferFromCart(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(cart_checkout, MultipleItemsCheckout(mOfferProvider->GetOffersInCart()))
    HANDLE_ACTION(lock_cart, LockCart())
    HANDLE_ACTION(unlock_cart, UnlockCart())
    HANDLE_EXPR(is_curr_filter_cart, IsCurrFilterCart(_msg->Int(2)))
    HANDLE_EXPR(is_cart_empty, mOfferProvider->NumOffersInCart() == 0)
    HANDLE_EXPR(is_cart_full, mOfferProvider->NumOffersInCart() == 6)
    HANDLE_ACTION(empty_cart, EmptyCart())
    HANDLE_ACTION(set_filter_to_cart, SetFilterToCart())
    HANDLE_EXPR(set_filter_to_songs, SetFilterToSongs())
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
