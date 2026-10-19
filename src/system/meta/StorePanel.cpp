#include "meta/StorePanel.h"
#include "StorePreviewMgr.h"
#include "macros.h"
#include "meta/Profile.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "meta/StorePurchaser.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "stl/_algobase.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/BufStream.h"
#include "utl/JobMgr.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"
#include <list>

StorePanel::StorePanel()
    : mNeedsLoad(false), mLoadOK(false), mShowTestOffers(false), mPendingArtLoader(0),
      mAlbumTex(Hmx::Object::New<RndTex>()), mPendingArtCallback(), mCurrentEnumJobID(-1),
      mStorePreviewMgr(0), mNeedToEnumerate(false), mPurchaser(0),
      mPendingPurchase(nullptr, nullptr), mSource(gNullStr), mBackupSource(gNullStr),
      mSessionStatus(kSessionNone), mPendingReenumJob(0) {}

StorePanel::~StorePanel() {
    DeleteAll(mOffers);
    DeleteAll(mExtraOffers);
    delete mAlbumTex;
}

BEGIN_PROPSYNCS(StorePanel)
    SYNC_PROP(load_ok, mLoadOK)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void StorePanel::Load() {
    UIPanel::Load();
    mLoadOK = true;
    mNeedsLoad = true;

    ThePlatformMgr.AddSink(this);
    Profile *profile = StoreProfile();
    if (!profile) {
        if (mLoadOK) {
            mLoadOK = false;
            ExitStore(kStoreErrorSignedOut);
        }
    } else if (!ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
        if (mLoadOK) {
            mLoadOK = false;
            ExitStore(kStoreErrorLiveServer);
        }
    }
    TheContentMgr.StartRefresh();
    MILO_ASSERT(!mStorePreviewMgr, 0xad);
    mStorePreviewMgr = new StorePreviewMgr();
    mStorePreviewMgr->AddSink(this);
    MILO_ASSERT(!mPurchaser, 0xb1);
    mSessionStatus = kSessionCreated;
}

void StorePanel::Enter() {
    UIPanel::Enter();
    Profile *profile = StoreProfile();
    if (!profile) {
        if (mLoadOK) {
            mLoadOK = false;
            ExitStore(kStoreErrorSignedOut);
        }

    } else if (!ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())
               || ThePlatformMgr.IsPadAGuest(profile->GetPadNum())) {
        if (mLoadOK) {
            mLoadOK = false;
            ExitStore(kStoreErrorLiveServer);
        }
    }

    if (mNeedsLoad) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        mNeedsLoad = false;
    }
    mShowing = mLoadOK;
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
    mNeedToEnumerate = false;
}

void StorePanel::Exit() {
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
    ThePlatformMgr.RemoveSink(this);
    if (0 <= mCurrentEnumJobID)
        ThePlatformMgr.CancelEnumJob(mCurrentEnumJobID);
    mCurrentEnumJobID = -1;
    UIPanel::Exit();
}

bool StorePanel::Exiting() const {
    if (mPurchaser && mPurchaser->IsPurchasing()) {
        return true;
    }

    return UIPanel::Exiting();
}

bool StorePanel::IsLoaded() const {
    return (UIPanel::IsLoaded() && TheContentMgr.RefreshDone());
}

void StorePanel::Unload() {
    if (0 <= mCurrentEnumJobID) {
        ThePlatformMgr.CancelEnumJob(mCurrentEnumJobID);
    }
    mCurrentEnumJobID = -1;
    RELEASE(mPurchaser);
    mPendingPurchase.first = nullptr;
    mPendingPurchase.second = nullptr;
    mPendingPurchases.clear();
    mStorePreviewMgr->RemoveSink(this);
    RELEASE(mStorePreviewMgr);
    FOREACH (it, mLoaders) {
        TheNetCacheMgr->DeleteNetCacheLoader(*it);
    }
    mLoaders.clear();
    DeleteAll(mOffers);
    DeleteAll(mExtraOffers);
    TheNetCacheMgr->Unload();
    UIPanel::Unload();
}

void StorePanel::CheckOut(StorePurchaseable *p) {
    MILO_ASSERT(p->IsAvailable(), 0x2c0);
    MILO_ASSERT(!mPurchaser, 0x2c1);
    Profile *profile = StoreProfile();
    MILO_ASSERT(profile, 0x2c4);
    mPendingPurchase.first = p;
    mPendingPurchase.second = profile;
    mPurchaser = new XboxPurchaser(profile->GetPadNum(), p->OfferID(), 0, 0, mSource, 0);
    mPurchaser->Initiate();
}

void StorePanel::ExitError(StoreError e) {
    MILO_ASSERT(e != kStoreErrorSuccess, 0x405);
    if (mLoadOK) {
        mLoadOK = false;
        ExitStore(e);
    }
}

void StorePanel::HandleNetCacheLoaderFailure(int failType) {
    MILO_ASSERT_RANGE(failType, 0, kNCMFT_Max, 0xe5);
    StoreError error;
    if (failType == 0) {
        failType = TheNetCacheMgr->GetFailType();
    }
    switch (failType) {
    case 1: {
        error = !ThePlatformMgr.IsSignedIntoLive(StoreProfile()->GetPadNum())
            ? kStoreErrorLiveServer
            : kStoreErrorStoreServer;
        break;
    }
    case 2:
        return;
    case 3:
        error = kStoreErrorNoEthernetCable;
        goto lol;
        break;
    default:
        MILO_NOTIFY("Unknown failure %d in a net cache loader!", failType);
        error = kStoreErrorStoreServer;
        break;
    }
    if (!ThePlatformMgr.IsEthernetCableConnected()) {
        error = kStoreErrorNoEthernetCable;
    }
lol:
    ExitError(error);
}

void StorePanel::HandleNetCacheMgrFailure() {
    StoreError error = kStoreErrorStoreServer;
    NetCacheMgrFailType failType = TheNetCacheMgr->GetFailType();

    switch (failType) {
    case kNCMFT_StoreServer:
    case kNCMFT_ClientError: {
        error = !ThePlatformMgr.IsSignedIntoLive(StoreProfile()->GetPadNum())
            ? kStoreErrorLiveServer
            : kStoreErrorStoreServer;
        break;
    }
    case 3:
        error = kStoreErrorNoEthernetCable;
        goto lol;
        break;
    default:
        MILO_NOTIFY("Unknown failure %d in NetCacheMgr.", failType);
        break;
    }
    if (!ThePlatformMgr.IsEthernetCableConnected()) {
        error = kStoreErrorNoEthernetCable;
    }
lol:
    ExitError(error);
}

void StorePanel::MultipleItemsCheckout(std::list<StoreOffer *> *offers) {
    MILO_ASSERT(!mPurchaser, 0x2e7);
    Profile *profile = StoreProfile();
    MILO_ASSERT(profile, 0x2ea);
    std::vector<unsigned long long> songIDs;
    FOREACH (it, *offers) {
        MILO_ASSERT((*it)->IsAvailable(), 0x2ef);
        songIDs.push_back((*it)->OfferID());
        mPendingPurchases.push_back(
            std::pair<StorePurchaseable *, const Profile *>(*it, profile)
        );
    }
    mPurchaser =
        new XboxMultipleItemsPurchaser(profile->GetPadNum(), songIDs, mSource, 0);
    mPurchaser->Initiate();
}

void StorePanel::PopulateOffers(DataArray *offers, bool extra) {
    std::vector<StoreOffer *> *container;
    if (mLoadOK) {
        DeleteAll(mExtraOffers);
        if (!extra) {
            DeleteAll(mOffers);
        }

        container = extra ? &mExtraOffers : &mOffers;

        if (offers) {
            offers->AddRef();
            for (int i = 1; i < offers->Size(); i++) {
                StoreOffer *newOffer = MakeNewOffer(offers->Array(i));
                if ((!mShowTestOffers && newOffer->IsTest())) {
                    delete newOffer;
                } else if (newOffer->ValidTitle()) {
                    container->push_back(newOffer);
                }
            }
            ValidateOffers(*container);
            offers->Release();
        }
    }
}

void StorePanel::EnumerateOffers(bool b) {
    Profile *profile = StoreProfile();
    MILO_ASSERT(profile, 0x356);
    Job *job;
    if (EnumerateSubsetOfOfferIDs()) {
        std::vector<UINT64> ids;
        GetOfferIDsToEnumerate(ids, b);
        if (ids.empty()) {
            if (mLoadOK) {
                mLoadOK = false;
                ExitStore(kStoreErrorNoMetadata);
            }
            return;
        }
        job = new StoreEnumJob(this, profile->GetPadNum(), &ids);
    } else {
        job = new StoreEnumJob(this, profile->GetPadNum(), nullptr);
    }
    ThePlatformMgr.QueueEnumJob(job);
    mCurrentEnumJobID = job->ID();
    static Message msg("enum_start");
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void StorePanel::FinishEnum(std::list<EnumProduct> const &enumProducts, bool b) {
    mCurrentEnumJobID = -1;
    if (b) {
        StoreError error = UpdateOffers(enumProducts, false);
        if (error == kStoreErrorSuccess || error == kStoreErrorNoContent) {
            if (!mExtraOffers.empty()) {
                error = UpdateOffers(enumProducts, true);
            }
        }
        if (error != kStoreErrorSuccess) {
            if (error == kStoreErrorNoContent) {
                if (!TheNetCacheMgr->IsDebug()) {
                    MILO_NOTIFY(
                        "No offers in this metadata were found in the enumeration!"
                    );
                } else {
                    goto lmao;
                }
            }
            ExitError(error);
        } else {
        lmao:
            static Message msg("enum_finished");
            HandleType(msg);
            TheUI->Handle(msg, false);
        }
    } else {
        MILO_NOTIFY("An enumeration failed!");
        if (mLoadOK) {
            mLoadOK = false;
            ExitStore(kStoreErrorLiveServer);
        }
    }
}

void StorePanel::UpdateFromEnumProduct(StorePurchaseable *sp, EnumProduct const *ep) {
    MILO_ASSERT(sp, 0x3f0);
    MILO_ASSERT(ep, 0x3f1);
    sp->SetPurchased(ep->mPurchased != 0);
    sp->SetPointsPrice(ep->mPrice);
    sp->SetAvailable(true);
}

void StorePanel::StartReEnum() {
    if (mPendingReenumJob != 0) {
        ThePlatformMgr.QueueEnumJob(mPendingReenumJob);
        mPendingReenumJob = nullptr;
    }
}

void StorePanel::LoadArt(const char *c, UIPanel *panel) {
    String str(c);
    auto it = std::find(mLoaders.begin(), mLoaders.end(), str);
    if (it == mLoaders.end()) {
        NetCacheLoader *loader = TheNetCacheMgr->AddNetCacheLoader(c, (NetLoaderPos)0);
        mPendingArtLoader = loader;
        if (loader) {
            mLoaders.push_back(mPendingArtLoader);
        }
    } else {
        mPendingArtLoader = *it;
    }
    mPendingArtCallback = panel;
}

void StorePanel::Poll() {
    UIPanel::Poll();
    if (mLoadOK) {
        if (TheNetCacheMgr->HasFailed()) {
            HandleNetCacheMgrFailure();
            return;
        }
        if (TheNetCacheMgr->IsReady()) {
            mStorePreviewMgr->Poll();
            {
                NetCacheMgrFailType fail;
                if (mStorePreviewMgr->GetLastFailure(fail)) {
                    HandleNetCacheLoaderFailure(fail);
                }
            }
            auto it = mLoaders.begin();
            while (it != mLoaders.end()) {
                NetCacheLoader *loader = *it;
                if (loader->IsLoaded()) {
                    if (loader == mPendingArtLoader) {
                        MILO_ASSERT(mPendingArtCallback, 0x167);
                        int size = loader->GetSize();
                        char *pBuffer = loader->GetBuffer();
                        MILO_ASSERT(pBuffer, 0x16d);
                        RndBitmap bitmap;
                        BufStream stream(pBuffer, size, true);
                        bitmap.Load(stream);
                        bitmap.SetMip(nullptr);
                        TheNetCacheMgr->DeleteNetCacheLoader(loader);
                        mAlbumTex->SetBitmap(bitmap, 0, false, RndTex::kRegular);
                        if (mPendingArtCallback->GetState() == kUp) {
                            static Message msg("art_loaded");
                            mPendingArtCallback->Handle(msg, false);
                        }
                        mPendingArtLoader = nullptr;
                        mPendingArtCallback = nullptr;
                    } else {
                        TheNetCacheMgr->DeleteNetCacheLoader(loader);
                    }
                    it = mLoaders.erase(it);
                } else if (loader->HasFailed()) {
                    NetCacheMgrFailType f = loader->GetFailType();
                    TheNetCacheMgr->DeleteNetCacheLoader(loader);
                    it = mLoaders.erase(it);
                    HandleNetCacheLoaderFailure(f);
                } else {
                    ++it;
                }
            }
            if (!mPurchaser && mNeedToEnumerate && mCurrentEnumJobID == -1) {
                mNeedToEnumerate = false;
                EnumerateOffers(mExtraOffers.empty() == 0);
            }
            if (mPurchaser) {
                mPurchaser->Poll();
                if (!mPurchaser->IsPurchasing()) {
                    bool b1 = false;
                    bool b2 = false;
                    if (mPurchaser->IsSuccess()) {
                        if (mPendingPurchases.size() != 0) {
                            if (mPurchaser->NeedsEnum()) {
                                std::vector<unsigned long long> ids;
                                for (int i = 0; i < mPendingPurchases.size(); i++) {
                                    ids.push_back(mPendingPurchases[i].first->OfferID());
                                }
                                mPendingReenumJob = new MultipleItemsPostPurchaseEnumJob(
                                    this,
                                    mPendingPurchases.front().second->GetPadNum(),
                                    ids,
                                    mPurchaser->Source(),
                                    mPurchaser->PID()
                                );
                                b2 = true;
                            }
                        } else {
                            if (mPendingPurchase.first
                                && !mPendingPurchase.first->IsPurchased()) {
                                if (mPurchaser->PurchaseMade()) {
                                    b1 = true;
                                    mPendingPurchase.first->SetPurchased(true);
                                    static Message msg("enum_finished");
                                    HandleType(msg);
                                    TheUI->Handle(msg, false);
                                } else {
                                    if (mPurchaser->NeedsEnum()
                                        && mPendingPurchase.second) {
                                        mPendingReenumJob = new PostPurchaseEnumJob(
                                            this,
                                            mPendingPurchase.second->GetPadNum(),
                                            mPendingPurchase.first->OfferID(),
                                            mPurchaser->Source(),
                                            mPurchaser->PID()
                                        );
                                        b2 = true;
                                    }
                                }
                            }
                        }
                    }
                    static Message msg("checkout_finished", 0, 0);
                    msg[0] = b1;
                    msg[1] = b2;
                    HandleType(msg);
                    TheUI->Handle(msg, false);
                    RELEASE(mPurchaser);
                    mPendingPurchase.first = nullptr;
                    mPendingPurchase.second = nullptr;
                    mPendingPurchases.clear();
                }
            }
        }
    }
}

DataNode StorePanel::OnMsg(SigninChangedMsg const &msg) {
    Profile *profile = StoreProfile();
    if (profile) {
        int changedMask = msg.GetChangedMask();
        int padnum = profile->GetPadNum();
        // idk what this means but ghidra said so
        if (((1 << padnum) & changedMask) == 0) {
            return DataNode(kDataInt, 1);
        }
    }
    if (mLoadOK) {
        mLoadOK = false;
        ExitStore(kStoreErrorSignedOut);
    }
    return DataNode(kDataInt, 1);
}

DataNode StorePanel::OnMsg(ProfileSwappedMsg const &) { return 1; }

DataNode StorePanel::OnMsg(SingleItemEnumCompleteMsg const &msg) {
    bool ok = msg.Success() && msg.HasOfferID();
    if (ok) {
        unsigned long long id = msg.OfferID();
        FOREACH (it, mOffers) {
            StoreOffer *offer = *it;
            if (offer->OfferID() == id) {
                offer->SetPurchased(true);
                static Message enumMsg("enum_finished");
                HandleType(enumMsg);
                TheUI->Handle(enumMsg, false);
                break;
            }
        }
    }

    static Message doneMsg("reenum_finished", 0);
    doneMsg[0] = ok;
    TheUI->Handle(doneMsg, false);
    return 0;
}

DataNode StorePanel::OnMsg(MultipleItemsEnumCompleteMsg const &msg) {
    bool success = msg.Success();
    if (success) {
        int numOffers = msg.NumOffers();
        for (int i = 0; i < numOffers; i++) {
            unsigned long long id = msg.OfferID(i);
            FOREACH (it, mOffers) {
                StoreOffer *offer = *it;
                if (offer->OfferID() == id) {
                    if (!offer->IsPurchased()) {
                        offer->SetPurchased(msg.Purchased(i));
                    }
                    break;
                }
            }
        }
        static Message enumMsg("enum_finished");
        HandleType(enumMsg);
        TheUI->Handle(enumMsg, false);
    }
    static Message doneMsg("reenum_finished", 0);
    doneMsg[0] = success;
    TheUI->Handle(doneMsg, false);
    return 0;
}

BEGIN_HANDLERS(StorePanel)
    HANDLE_EXPR(toggle_test_offers, ToggleTestOffers())
    HANDLE_EXPR(test_offers, mShowTestOffers)
    HANDLE_ACTION(load_art, LoadArt(_msg->Str(2), _msg->Obj<UIPanel>(3)))
    HANDLE_EXPR(album_tex, mAlbumTex)
    HANDLE_ACTION(cancel_art, CancelArt())
    HANDLE_ACTION(check_out, CheckOut(_msg->Obj<StorePurchaseable>(2)))
    HANDLE_ACTION(re_download, CheckOut(_msg->Obj<StorePurchaseable>(2)))
    HANDLE_ACTION(set_source, SetSource(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(set_source_to_backup, SetSourceToBackup())
    HANDLE_ACTION(start_reenum_if_needed, StartReEnum())
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(ProfileSwappedMsg)
    HANDLE_MESSAGE(SingleItemEnumCompleteMsg)
    HANDLE_MESSAGE(MultipleItemsEnumCompleteMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

StoreEnumJob::StoreEnumJob(StorePanel *panel, int i, std::vector<UINT64> *vec) {
    mEnum = new XboxEnumeration(i, vec);
    mStorePanel = panel;
}

StoreEnumJob::~StoreEnumJob() { delete mEnum; }

bool StoreEnumJob::IsFinished() {
    if (mEnum->IsEnumerating()) {
        mEnum->Poll();
    }
    return mEnum->IsEnumerating() == false;
}

void StoreEnumJob::Start() { mEnum->Start(); }
