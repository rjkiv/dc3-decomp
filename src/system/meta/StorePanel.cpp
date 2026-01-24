#include "meta/StorePanel.h"
#include "macros.h"
#include "meta/Profile.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

StorePanel::StorePanel()
    : unk50(false), mLoadOk(false), unk52(false), unk5c(0),
      unk60(Hmx::Object::New<RndTex>()), mPendingArtCallback(0), unk68(-1),
      mStorePreviewMgr(0), unk70(false), mPurchaser(0), unk78(nullptr), unk7c(0),
      unk8c(gNullStr), unk90(gNullStr), unk94(0), unk98(0) {}

StorePanel::~StorePanel() {}

BEGIN_PROPSYNCS(StorePanel)
    SYNC_PROP(load_ok, mLoadOk)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void StorePanel::Load() {}

void StorePanel::Enter() {}

void StorePanel::Exit() {
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
    ThePlatformMgr.RemoveSink(this, gNullStr);
    if (0 <= unk68)
        ThePlatformMgr.CancelEnumJob(unk68);
    unk68 = -1;
    UIPanel::Exit();
}

bool StorePanel::Exiting() const {
    bool b;
    if (mPurchaser && mPurchaser->unk8 != 0) {
        b = UIPanel::Exiting();
    }
    b = true;

    return b;
}

void StorePanel::Poll() {}

bool StorePanel::IsLoaded() const {
    return (UIPanel::IsLoaded() && TheContentMgr.RefreshDone());
}

void StorePanel::Unload() {
    if (0 < unk68) {
        ThePlatformMgr.CancelEnumJob(unk68);
    }
    unk68 = -1;
    RELEASE(mPurchaser);
    unk78 = 0;
    unk7c = 0;
    unk80.clear();
    RemoveSink(mStorePreviewMgr, gNullStr);
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

void StorePanel::LoadArt(char const *, UIPanel *) {}

void StorePanel::CheckOut(StorePurchaseable *p) {
    MILO_ASSERT(p->IsAvailable(), 0x2c0);
    MILO_ASSERT(!mPurchaser, 0x2c1);
}

void StorePanel::ExitError(StoreError e) {
    MILO_ASSERT(e != kStoreErrorSuccess, 0x405);
    if (mLoadOk) {
        mLoadOk = false;
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
    // MILO_ASSERT(profile, 0x2ea);
    FOREACH (it, *offers) {
        MILO_ASSERT((*it)->IsAvailable(), 0x2ef);
    }
}

void StorePanel::PopulateOffers(DataArray *, bool) {}

void StorePanel::EnumerateOffers(bool) {}

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

DataNode StorePanel::OnMsg(SigninChangedMsg const &) { return 0; }

DataNode StorePanel::OnMsg(ProfileSwappedMsg const &) { return 0; }

void StorePanel::ValidateOffers(std::vector<StoreOffer *> &) {}

BEGIN_HANDLERS(StorePanel)
END_HANDLERS

StoreEnumJob::StoreEnumJob(StorePanel *panel, int i, std::vector<UINT64> *vec) {
    mEnumeration = new XboxEnumeration(i, vec);
    mStorePanel = panel;
}

StoreEnumJob::~StoreEnumJob() {
    delete mEnumeration;
}

bool StoreEnumJob::IsFinished() {
    if (mEnumeration->IsEnumerating()) {
        mEnumeration->Poll();
    }
    return mEnumeration->IsEnumerating() == false;
}