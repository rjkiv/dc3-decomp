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
#include "rndobj/Tex.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/JobMgr.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"
#include <list>

StorePanel::StorePanel()
    : unk50(false), mLoadOk(false), unk52(false), unk5c(0),
      unk60(Hmx::Object::New<RndTex>()), mPendingArtCallback(0), unk68(-1),
      mStorePreviewMgr(0), unk70(false), mPurchaser(0), unk78(nullptr), unk7c(nullptr),
      unk8c(gNullStr), unk90(gNullStr), unk94(0), unk98(0) {}

StorePanel::~StorePanel() {
    DeleteAll(unk38);
    DeleteAll(unk44);
    delete unk60;
}

BEGIN_PROPSYNCS(StorePanel)
    SYNC_PROP(load_ok, mLoadOk)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void StorePanel::Load() {
    UIPanel::Load();
    unk50 = true;
    // mLoadOk = true;
    ThePlatformMgr.AddSink(this);
    Profile *profile = StoreProfile();
    if (!profile) {
        if (mLoadOk) {
            mLoadOk = false;
            ExitError(kStoreErrorLiveServer);
        }
    } else if (!ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
        if (mLoadOk) {
            mLoadOk = false;
            ExitError(kStoreErrorCacheNoSpace);
        }
    }
    TheContentMgr.StartRefresh();
    MILO_ASSERT(!mStorePreviewMgr, 0xad);
    mStorePreviewMgr = new StorePreviewMgr();
    mStorePreviewMgr->AddSink(this);
    MILO_ASSERT(!mPurchaser, 0xb1);
    unk94 = 2;
}

void StorePanel::Enter() {
    UIPanel::Enter();
    Profile *profile = StoreProfile();
    if (!profile) {
        if (mLoadOk) {
            mLoadOk = false;
            ExitStore(kStoreErrorLiveServer);
        }
    } else {
        if (ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())
            && ThePlatformMgr.IsPadAGuest(profile->GetPadNum())) {
            if (mLoadOk) {
                mLoadOk = false;
                ExitError(kStoreErrorCacheNoSpace);
            }
        }
    }

    if (unk50) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        unk50 = false;
    }
    mShowing = mLoadOk;
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
    unk70 = false;
}

void StorePanel::Exit() {
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
    ThePlatformMgr.RemoveSink(this);
    if (0 <= unk68)
        ThePlatformMgr.CancelEnumJob(unk68);
    unk68 = -1;
    UIPanel::Exit();
}

bool StorePanel::Exiting() const {
    if (mPurchaser && mPurchaser->IsPurchasing()) {
        return true;
    }

    return UIPanel::Exiting();
}

void StorePanel::Poll() {}

bool StorePanel::IsLoaded() const {
    return (UIPanel::IsLoaded() && TheContentMgr.RefreshDone());
}

void StorePanel::Unload() {
    if (0 <= unk68) {
        ThePlatformMgr.CancelEnumJob(unk68);
    }
    unk68 = -1;
    RELEASE(mPurchaser);
    unk78 = nullptr;
    unk7c = nullptr;
    unk80.clear();
    RemoveSink(mStorePreviewMgr);
    RELEASE(mStorePreviewMgr);
    FOREACH (it, unk54) {
        TheNetCacheMgr->DeleteNetCacheLoader(*it);
    }
    unk54.clear();
    DeleteAll(unk38);
    DeleteAll(unk44);
    TheNetCacheMgr->Unload();
    UIPanel::Unload();
}

void StorePanel::LoadArt(char const *c, UIPanel *panel) {}

void StorePanel::CheckOut(StorePurchaseable *p) {
    MILO_ASSERT(p->IsAvailable(), 0x2c0);
    MILO_ASSERT(!mPurchaser, 0x2c1);
    Profile *profile = StoreProfile();
    MILO_ASSERT(profile, 0x2c4);
    unk78 = p;
    unk7c = profile;
    mPurchaser = new XboxPurchaser(profile->GetPadNum(), p->SongID(), 0, 0, unk8c, 0);
    mPurchaser->Initiate();
}

void StorePanel::ExitError(StoreError e) {
    MILO_ASSERT(e != kStoreErrorSuccess, 0x405);
    if (mLoadOk) {
        mLoadOk = false;
        ExitStore(e);
    }
}

void StorePanel::HandleNetCacheMgrFailure() {}

void StorePanel::HandleNetCacheLoaderFailure(int failType) {
    MILO_ASSERT((0) <= (failType) && (failType) < (kNCMS_Max), 0xe5);
    switch (failType) {
    case 0:

        break;
    }
}

void StorePanel::MultipleItemsCheckout(std::list<StoreOffer *> *offers) {
    MILO_ASSERT(!mPurchaser, 0x2e7);
    Profile *profile = StoreProfile();
    MILO_ASSERT(profile, 0x2ea);
    std::vector<unsigned long long> songIDs;
    FOREACH (it, *offers) {
        MILO_ASSERT((*it)->IsAvailable(), 0x2ef);
        songIDs.push_back((*it)->SongID());
        unk80.push_back(std::pair<StorePurchaseable *, const Profile *>(*it, profile));
    }
    mPurchaser = new XboxMultipleItemsPurchaser(profile->GetPadNum(), songIDs, unk8c, 0);
    mPurchaser->Initiate();
}

void StorePanel::PopulateOffers(DataArray *d, bool b) {
    if (mLoadOk) {
        DeleteAll(unk44);
        if (!b) {
            DeleteAll(unk38);
        }
        std::vector<StoreOffer *> *offers = &unk44;
        if (!b) {
            offers = &unk38;
        }
        if (d) {
            d->AddRef();
            for (int i = 1; i < d->Size(); i++) {
                StoreOffer *offer = MakeNewOffer(d->Array(i));
                if (!unk52 && offer->IsTest() && !offer->ValidTitle()) {
                    delete offer;
                } else if (offer->ValidTitle()) {
                    offers->push_back(offer);
                }
            }
            ValidateOffers(*offers);
            d->Release();
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
            if (mLoadOk) {
                mLoadOk = false;
                ExitStore(kStoreErrorSignedOut);
            }
            return;
        }
        job = new StoreEnumJob(this, profile->GetPadNum(), &ids);
    } else {
        job = new StoreEnumJob(this, profile->GetPadNum(), nullptr);
    }
    ThePlatformMgr.QueueEnumJob(job);
    unk68 = job->ID();
    static Message msg("enum_start");
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void StorePanel::FinishEnum(std::list<EnumProduct> const &, bool) {}

StoreError StorePanel::UpdateOffers(std::list<EnumProduct> const &, bool) {
    return kStoreErrorNoMetadata;
}

void StorePanel::UpdateFromEnumProduct(StorePurchaseable *sp, EnumProduct const *ep) {
    MILO_ASSERT(sp, 0x3f0);
    MILO_ASSERT(ep, 0x3f1);
    sp->isPurchased = (ep->unk10 != 0);
    sp->cost = ep->unk14;
    sp->isAvailable = true;
}

void StorePanel::StartReEnum() {
    if (unk98 != 0) {
        ThePlatformMgr.QueueEnumJob(unk98);
        unk98 = nullptr;
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
    if (mLoadOk) {
        mLoadOk = false;
        ExitStore(kStoreErrorLiveServer);
    }
    return DataNode(kDataInt, 1);
}

DataNode StorePanel::OnMsg(ProfileSwappedMsg const &) { return 1; }

DataNode StorePanel::OnMsg(SingleItemEnumCompleteMsg const &msg) {
    bool ok = msg.Success() && msg.HasOfferID();
    if (ok) {
        unsigned long long id = msg.OfferID();
        FOREACH (it, unk38) {
            StoreOffer *offer = *it;
            if (offer->SongID() == id) {
                offer->isPurchased = true;
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
            FOREACH (it, unk38) {
                StoreOffer *offer = *it;
                if (offer->SongID() == id) {
                    if (!offer->IsPurchased()) {
                        offer->isPurchased = msg.Purchased(i);
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
    HANDLE_EXPR(test_offers, unk52)
    HANDLE_ACTION(load_art, LoadArt(_msg->Str(2), _msg->Obj<UIPanel>(3)))
    HANDLE_EXPR(album_tex, unk60)
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
    mEnumeration = new XboxEnumeration(i, vec);
    mStorePanel = panel;
}

StoreEnumJob::~StoreEnumJob() { delete mEnumeration; }

bool StoreEnumJob::IsFinished() {
    if (mEnumeration->IsEnumerating()) {
        mEnumeration->Poll();
    }
    return mEnumeration->IsEnumerating() == false;
}

void StoreEnumJob::Start() { mEnumeration->Start(); }
