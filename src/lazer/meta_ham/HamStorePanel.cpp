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
#include "ui/UI.h"
#include "utl/JobMgr.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/NetCacheMgr.h"
#include "utl/NetLoader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <list>

static const char *sIndexString = "s_store_index_%s_%s.dtz";

HamStorePanel::HamStorePanel()
    : mMetadataLoader(), mMetadata(), mOfferProvider(), mMotd(), mAllowCancel(false),
      mReLockDuration(), mReadyToEnumCartOffer(false), mIsCartEnabled(true),
      mCartLocked(false), mCartLoaded(false), mRemovingDLCFromCart(false),
      mAddingDLCFromCart(false), mSpecialEnumJobID(-1), mSpecialPurchaser() {
    for (int i = 0; i < 7; i++) {
        mCartJobs[i] = 0;
    }
    DataArray *specialOfferArray =
        SystemConfig("store")->FindArray("special_offers", false);
    if (specialOfferArray) {
        int numOffers = specialOfferArray->Size() - 1;
        HamSpecialOffer tempOffer;
        mSpecialOffers.resize(numOffers, tempOffer);
        mSpecialOfferIDs.resize(numOffers, 0);

        for (int i = 0; i < numOffers; i++) {
            DataArray *offerArray = specialOfferArray->Array(i + 1);
            HamSpecialOffer &offer = mSpecialOffers[i];
            offer.mID = offerArray->Sym(0);
            offer.mIsOwned = false;
            offer.mOfferId = StorePurchaseable::OfferStringToID(offerArray->Str(1));
            mSpecialOfferIDs[i] = offer.mOfferId;
            offer.mLocalFileName = offerArray->ForceSym(2);
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
    mOfferProvider = new HamStoreProvider(&unk38, &mFilters, &mCartData);
    mAllowCancel = false;
    mCancelTimer.Restart();
    RefreshSpecialOfferStatus();
}

bool HamStorePanel::Exiting() const {
    if (mSpecialPurchaser != 0)
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
    if (mCartLoaded) {
        MILO_FAIL("Can\'t disable the cart after it is loaded");
    }
    mIsCartEnabled = false;
    mReLockDuration = 0;
}

void HamStorePanel::RemoveDLCFromCart(int id) {
    mPendingRemovals.push_back(id);
    if (!mRemovingDLCFromCart) {
        mRemovingDLCFromCart = true;
        RemoveNextDLCFromCart();
    }
}

void HamStorePanel::AddDLCToCart(int id) {
    mPendingAdds.push_back(id);
    if (!mAddingDLCFromCart) {
        mAddingDLCFromCart = true;
        AddNextDLCToCart();
    }
}

void HamStorePanel::RemoveOfferFromCart(StoreOffer *offer) {
    mOfferProvider->UpdateOffersInCart(offer, 1);
    RemoveDLCFromCart(offer->GetSingleSongID());
}

bool HamStorePanel::IsCurrFilterCart(int id) {
    static Symbol store_filter_shopping_cart("store_filter_shopping_cart");
    return mFilters[id]->GetName() == store_filter_shopping_cart;
}

void HamStorePanel::GetCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x23d);
    mCartJobs[3] = new GetCartJob(this, profile);
    TheRockCentral.ManageJob(mCartJobs[3]);
}

void HamStorePanel::LockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x246);
    mCartLocked = true;
    mReLockDuration = 0;
    mCartJobs[4] = new LockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(mCartJobs[4]);
}

void HamStorePanel::UnlockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x252);
    mCartLocked = false;
    mCartJobs[5] = new UnlockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(mCartJobs[5]);
}

void HamStorePanel::RelockCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x25d);
    mCartLocked = true;
    mCartJobs[6] = new LockCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(mCartJobs[6]);
    mReLockCartTimer.Restart();
}

void HamStorePanel::EmptyCart() {
    mOfferProvider->UpdateOffersInCart(nullptr, 2);
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x26c);
    mCartJobs[2] = new EmptyCartJob(this, profile->GetOnlineID()->ToString());
    TheRockCentral.ManageJob(mCartJobs[2]);
}

void HamStorePanel::StoreUserProfileSwappedToUser(LocalUser *) {
    RefreshSpecialOfferStatus();
}

void HamStorePanel::ReadLockData() {
    ((LockCartJob *)mCartJobs[4])->GetLockData(mReLockDuration);
    mCartJobs[4] = nullptr;
    mReLockCartTimer.Restart();
}

void HamStorePanel::ReadCartData() {
    GetCartJob *job = (GetCartJob *)mCartJobs[3];
    mCartData.clear();
    job->GetRows(&mCartData);
    mCartJobs[3] = nullptr;
    mCartLoaded = true;
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
    for (int i = mFilters.size() - 1; i >= 0; i--) {
        if (mFilters[i]->GetName() == store_filter_shopping_cart) {
            mOfferProvider->SetFilter(mFilters[i]);
            return;
        }
    }
}

int HamStorePanel::SetFilterToSongs() {
    static Symbol songs("songs");
    for (int i = mFilters.size() - 1; i >= 0; i--) {
        if (mFilters[i]->GetName() == songs) {
            mOfferProvider->SetFilter(mFilters[i]);
            return i;
        }
    }
    return -1;
}

void HamStorePanel::AddNextDLCToCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x2da);
    mCartJobs[0] = new AddDLCToCartJob(
        this, profile->GetOnlineID()->ToString(), mPendingAdds.front()
    );
    TheRockCentral.ManageJob(mCartJobs[0]);
}

void HamStorePanel::RemoveNextDLCFromCart() {
    HamProfile *profile = dynamic_cast<HamProfile *>(StoreProfile());
    MILO_ASSERT(profile, 0x2c0);
    mCartJobs[1] = new RemoveDLCFromCartJob(
        this, profile->GetOnlineID()->ToString(), mPendingRemovals.front()
    );
    TheRockCentral.ManageJob(mCartJobs[1]);
}

void HamStorePanel::AddOfferToCart(StoreOffer *offer) {
    mOfferProvider->UpdateOffersInCart(offer, 0);
    AddDLCToCart(offer->GetSingleSongID());
}

char const *HamStorePanel::GetIndexFile() const {
    Symbol platSym = PlatformSymbol(TheLoadMgr.GetPlatform());
    Symbol sysLang = SystemLanguage();
    return MakeString(sIndexString, platSym, sysLang);
}

void HamStorePanel::ExitStore(StoreError err) const {
    static Symbol store_load_failed("store_load_failed");
    if (TheUIEventMgr->CurrentEvent() != store_load_failed) {
        static Message msg("init", -1);
        msg[0] = err;
        TheUIEventMgr->TriggerEvent(store_load_failed, msg);
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
    auto it = mFilters.begin();
    mFilters.insert(it, new HamStoreFilter(store_filter_shopping_cart));
    mFilters.push_back(new HamStoreFilter(store_filter_song_import_offers));

    DataArrayPtr ptr;
    ptr->Insert(ptr->Size(), store_checkout);
    ptr->Insert(ptr->Size(), DataArrayPtr(type, fake));
    ptr->Insert(ptr->Size(), DataArrayPtr(name, ""));
    ptr->Insert(ptr->Size(), DataArrayPtr(artist, ""));
    ptr->Insert(ptr->Size(), DataArrayPtr(album_name, ""));
    ptr->Insert(ptr->Size(), DataArrayPtr(description, ""));
    ptr->Insert(ptr->Size(), DataArrayPtr(art, "avatar_theboombox_nomip_xbox.dxt"));
    unk38.push_back(MakeNewOffer(ptr));
}

bool HamStorePanel::IsSpecialOfferOwned(Symbol offer) const {
    FOREACH (it, mSpecialOffers) {
        if ((*it).mID == offer) {
            return (*it).mIsOwned;
        }
    }
    Symbol s = offer; // bruh
    MILO_NOTIFY("Unknown offer %s", s);
    return false;
}

void HamStorePanel::ResetCancelTimer() {
    mAllowCancel = false;
    mCancelTimer.Restart();
}

bool HamStorePanel::ContentTitleDiscovered(unsigned int ui, Symbol s) {
    static Symbol dc2_gond("dc2_gond");
    if (ui == 0x373307d2) {
        FOREACH (it, mSpecialOffers) {
            if (it->mID == dc2_gond) {
                it->mLocalFileName = s;
                break;
            }
        }
        return false;
    }
    return true;
}

void HamStorePanel::ContentMounted(char const *c1, char const *c2) {
    FOREACH (it, mSpecialOffers) {
        if (it->mLocalFileName == c1) {
            Symbol s = it->mID;
            MILO_LOG("Store: special offer %s on local drive.\n", s);
            it->mIsOwned = true;
            return;
        }
    }
}

bool HamStorePanel::ContentDiscovered(Symbol s) {
    FOREACH (it, mSpecialOffers) {
        if (it->mLocalFileName == s) {
            return false;
        }
    }
    return true;
}

bool HamStorePanel::BuySpecialOffer(Symbol offer) {
    if (mSpecialPurchaser) {
        MILO_FAIL("There is a purchase in progress.");
    }
    FOREACH (it, mSpecialOffers) {
        if ((*it).mID == offer) {
            Profile *profile = StoreProfile();
            if (profile) {
                mSpecialPurchaser = new XboxPurchaser(
                    profile->GetPadNum(), it->mOfferId, 0, 0, mSource, 0
                );
                mSpecialPurchaser->Initiate();
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
    if (!mAllowCancel && mCancelTimer.SplitMs() > 5000.0f) {
        mAllowCancel = true;
        if (TheHamUI.FocusPanel() == this) {
            TheHamUI.Helpbar()->SyncToPanel(this);
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
    if (mMetadataLoader) {
        mMetadataLoader->PollLoading();
        if (mMetadataLoader->IsLoaded()) {
            mMetadata = mMetadataLoader->GetData();
            MILO_ASSERT(mMetadata, 0xfe);
            mMetadata->AddRef();
            static Symbol motd("motd");
            mMetadata->FindData(motd, mMotd, false);
            DataArray *filterArray = mMetadata->FindArray("filters", true);
            DeleteAll(mFilters);
            for (int i = 1; i < filterArray->Size(); i++) {
                mFilters.push_back(new HamStoreFilter(filterArray->Array(i)));
            }
            DataArray *offerArray = mMetadata->FindArray("offers", true);
            PopulateOffers(offerArray, false);
            CreateCartUIs();
            mReadyToEnumCartOffer = true;
        } else {
            if (!mMetadataLoader->HasFailed()) {
                goto exit;
            }
            MILO_NOTIFY("Request for %s failed.", GetIndexFile());
            ExitError((StoreError)3);
        }
        RELEASE(mMetadataLoader);
    } else {
        if (mMetadata == 0) {
            String indexFile = GetIndexFile();
            mMetadataLoader = new DataNetLoader(indexFile);
        } else if (mReadyToEnumCartOffer && (mCartLoaded || !mIsCartEnabled)) {
            mReadyToEnumCartOffer = false;
            unk70 = true;
        }
    }
exit:
    if (mCartLocked && mReLockDuration != 0) {
        if (mReLockCartTimer.SplitMs() >= mReLockDuration) {
            RelockCart();
        }
    }
    if (mSpecialPurchaser) {
        mSpecialPurchaser->Poll();
        if (!mSpecialPurchaser->IsPurchasing()) {
            bool purchaseMade = false;
            bool needsEnum = false;
            if (mSpecialPurchaser->IsSuccess()) {
                purchaseMade = mSpecialPurchaser->PurchaseMade();
                needsEnum = mSpecialPurchaser->NeedsEnum();
                if (purchaseMade && needsEnum) {
                    RefreshSpecialOfferStatus();
                }
            }

            static Message msg("special_finished", 0, 0);
            msg[0] = purchaseMade;
            msg[1] = needsEnum;
            HandleType(msg);
            TheUI->Handle(msg, false);
            RELEASE(mSpecialPurchaser);
        }
    }
}

void HamStorePanel::Unload() {
    RELEASE(mMetadataLoader);
    RELEASE(mOfferProvider);
    if (mMetadata) {
        mMetadata->Release();
        mMetadata = nullptr;
    }
    DeleteAll(mFilters);
    mCancelTimer.Stop();
    mAllowCancel = false;
    mReLockCartTimer.Stop();
    mReLockDuration = 0;
    mIsCartEnabled = true;
    mReadyToEnumCartOffer = false;
    mCartLoaded = false;
    mRemovingDLCFromCart = false;
    mAddingDLCFromCart = false;
    if (mCartLocked) {
        UnlockCart();
    }
    StorePanel::Unload();
}

void HamStorePanel::FinishSpecialOfferEnum(std::vector<bool> const &vec, bool b) {
    mSpecialEnumJobID = -1;
    if (!b) {
        MILO_LOG("Store: failed to enum our special offers.\n");
    } else {
        for (int i = 0; i < mSpecialOffers.size(); i++) {
            HamSpecialOffer &offer = mSpecialOffers[i];
            if (!offer.mIsOwned) {
                offer.mIsOwned = vec[i];
            }

            if (offer.mIsOwned) {
                MILO_LOG("Store: special offer %s is owned\n", offer.mID);
            }
        }
    }
    static Message self_msg("refresh_complete", 0);
    self_msg[0] = b;
    TheUI->Handle(self_msg, false);
}

void HamStorePanel::RefreshSpecialOfferStatus() {
    Profile *pProfile = StoreProfile();
    if (pProfile && mSpecialEnumJobID == -1) {
        SpecialOfferEnumJob *job =
            new SpecialOfferEnumJob(this, pProfile->GetPadNum(), mSpecialOfferIDs);
        ThePlatformMgr.QueueEnumJob(job);
        mSpecialEnumJobID = job->ID();
    }
}

void HamStorePanel::GetOfferIDsToEnumerate(std::vector<QWORD> &offerIDs, bool b) const {
    const std::vector<StoreOffer *> &offers = b ? unk44 : unk38;
    for (int i = 0; i < offers.size(); i++) {
        StorePurchaseable *purchaseable = offers[i];
        if (purchaseable->Exists()) {
            offerIDs.push_back(purchaseable->SongID());
        }
    }
    std::sort(offerIDs.begin(), offerIDs.end());
    auto it = std::unique(offerIDs.begin(), offerIDs.end());
    offerIDs.resize(it - offerIDs.begin());
}

DataNode HamStorePanel::OnMsg(RCJobCompleteMsg const &msg) {
    if (msg.Job() == mCartJobs[4]) {
        if (msg.Success()) {
            ReadLockData();
            GetCart();
        } else {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to lock, disabling.\n");
            DisableCart();
        }
    } else if (msg.Job() == mCartJobs[5]) {
        if (msg.Success()) {
            MILO_LOG("[HamStorePanel::OnMsg] Cart unlocked successfully.\n");
        } else {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to unlock.\n");
        }
    } else if (msg.Job() == mCartJobs[6]) {
        if (!msg.Success()) {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to re-lock.\n");
        }
    } else if (msg.Job() == mCartJobs[3]) {
        if (msg.Success()) {
            ReadCartData();
        } else {
            MILO_LOG("[HamStorePanel::OnMsg] Failed to get cart, disabling.\n");
            DisableCart();
        }
    } else if (msg.Job() == mCartJobs[1]) {
        if (!msg.Success()) {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to remove song.\n");
            ExitError(kStoreErrorCacheRemoved);
        } else {
            mPendingRemovals.pop_front();
            if (!mPendingRemovals.empty()) {
                RemoveNextDLCFromCart();
            } else {
                mCartJobs[1] = nullptr;
                mRemovingDLCFromCart = false;
            }
        }
    } else if (msg.Job() == mCartJobs[0]) {
        if (!msg.Success()) {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to add song.\n");
            ExitError(kStoreErrorCacheRemoved);
        } else {
            mPendingAdds.pop_front();
            if (!mPendingAdds.empty()) {
                AddNextDLCToCart();
            } else {
                mCartJobs[0] = nullptr;
                mAddingDLCFromCart = false;
            }
        }
    } else if (msg.Job() == mCartJobs[2]) {
        if (!msg.Success()) {
            MILO_LOG("[HamStorePanel::OnMsg] Cart failed to clear.\n");
        } else {
            MILO_LOG("[HamStorePanel::OnMsg] Cart emptied successfully.\n");
        }
    }
    return 1;
}

StoreError HamStorePanel::UpdateOffers(std::list<EnumProduct> const &list, bool b) {
    FOREACH (it, mSpecialOffers) {
        if (!it->mIsOwned) {
            bool check = false;
            FOREACH (listIt, list) {
                if (listIt->unk8 == it->mOfferId) {
                    check = true;
                    it->mIsOwned = (listIt->unk10 != 0);
                    break;
                }
            }

            if (check) {
                MILO_LOG(
                    "Store: special offer %s is %s\n",
                    it->mID.Str(),
                    (it->mIsOwned) ? "owned" : "not owned"
                );
            }
        }
    }
    return StorePanel::UpdateOffers(list, b);
}

BEGIN_HANDLERS(HamStorePanel)
    HANDLE_EXPR(get_motd, mMotd)
    HANDLE_ACTION(set_filter, mOfferProvider->SetFilter(mFilters[_msg->Int(2)]))
    HANDLE_ACTION(
        set_filter_pack_singles, mOfferProvider->SetFilter(_msg->Obj<StoreOffer>(2))
    )
    HANDLE_EXPR(offer_provider, (Hmx::Object *)mOfferProvider)
    HANDLE_EXPR(filter_provider, (Hmx::Object *)mOfferProvider->GetFilterProvider())
    HANDLE_ACTION(reset_cancel_timer, ResetCancelTimer())
    HANDLE_EXPR(allow_cancel, mAllowCancel)
    HANDLE_EXPR(is_cart_enabled, mIsCartEnabled)
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

SpecialOfferEnumJob::SpecialOfferEnumJob(
    HamStorePanel *panel, int i, std::vector<QWORD> &vec
)
    : MultipleItemsEnumJob(nullptr, i, vec), mStorePanel(panel) {}
