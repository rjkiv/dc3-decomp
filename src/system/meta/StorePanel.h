#pragma once
#include "StorePurchaser.h"
#include "meta/Profile.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "meta/StorePreviewMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "rndobj/Tex.h"
#include "stl/_pair.h"
#include "types.h"
#include "ui/UIPanel.h"
#include "utl/NetCacheLoader.h"
#include "utl/Symbol.h"
#include <list>

class StorePanel : public UIPanel {
public:
    enum SessionStatus {
        kSessionNone = 0x0000,
        kSessionCreating = 0x0001,
        kSessionCreated = 0x0002
    };

    // Hmx::Object
    virtual ~StorePanel();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Load();

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual bool Exiting() const;
    virtual bool Unloading() const;
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual bool IsSongInLibrary(int const &) const = 0;
    virtual void ExitStore(StoreError) const = 0;
    virtual Profile *StoreProfile() const = 0;
    virtual StoreOffer *MakeNewOffer(DataArray *) = 0;
    virtual StoreOffer *FindOffer(Symbol) const = 0;
    virtual bool EnumerateSubsetOfOfferIDs() const { return false; };
    virtual void
    GetOfferIDsToEnumerate(std::vector<unsigned long long> &, bool) const = 0;
    virtual void LoadArt(char const *, UIPanel *);

    StorePanel();
    void CheckOut(StorePurchaseable *);
    void ExitError(StoreError);
    void HandleNetCacheMgrFailure();
    void HandleNetCacheLoaderFailure(int);
    void MultipleItemsCheckout(std::list<StoreOffer *> *);
    bool ToggleTestOffers() {
        mShowTestOffers = !mShowTestOffers;
        return mShowTestOffers;
    }
    void CancelArt() {
        mPendingArtLoader = 0;
        mPendingArtCallback = nullptr;
    }
    void SetSource(Symbol s, bool b) {
        mSource = s;
        if (b) {
            mBackupSource = s;
        }
    }
    void SetSourceToBackup() { mSource = mBackupSource; }

protected:
    // UIPanel
    virtual void PopulateOffers(DataArray *, bool);
    virtual void EnumerateOffers(bool);
    virtual void FinishEnum(std::list<EnumProduct> const &, bool);
    virtual StoreError UpdateOffers(std::list<EnumProduct> const &, bool);
    virtual void UpdateFromEnumProduct(StorePurchaseable *, EnumProduct const *);
    virtual void StoreUserProfileSwappedToUser(LocalUser *) = 0;

    void StartReEnum();
    DataNode OnMsg(SigninChangedMsg const &);
    DataNode OnMsg(ProfileSwappedMsg const &);
    void ValidateOffers(std::vector<StoreOffer *> &);
    DataNode OnMsg(SingleItemEnumCompleteMsg const &);
    DataNode OnMsg(MultipleItemsEnumCompleteMsg const &);

    std::vector<StoreOffer *> mOffers; // 0x38
    std::vector<StoreOffer *> mExtraOffers; // 0x44
    bool mNeedsLoad; // 0x50
    bool mLoadOK; // 0x51
    bool mShowTestOffers; // 0x52
    std::list<NetCacheLoader *> mLoaders; // 0x54
    NetCacheLoader *mPendingArtLoader; // 0x5c
    RndTex *mAlbumTex; // 0x60
    UIPanel *mPendingArtCallback; // 0x64
    int mCurrentEnumJobID; // 0x68
    StorePreviewMgr *mStorePreviewMgr; // 0x6c
    bool mNeedToEnumerate; // 0x70
    StorePurchaser *mPurchaser; // 0x74
    std::pair<StorePurchaseable *, const Profile *> mPendingPurchase; // 0x78
    std::vector<std::pair<StorePurchaseable *, const Profile *> > mPendingPurchases; // 0x80
    Symbol mSource; // 0x8c
    Symbol mBackupSource; // 0x90
    SessionStatus mSessionStatus; // 0x94
    Job *mPendingReenumJob; // 0x98
};

class StoreEnumJob : public Job {
public:
    StoreEnumJob(StorePanel *, int, std::vector<UINT64> *);
    virtual ~StoreEnumJob();
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);

private:
    XboxEnumeration *mEnum; // 0x8
    StorePanel *mStorePanel; // 0xc
};
