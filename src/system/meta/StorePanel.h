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
#include "stl/_vector.h"
#include "types.h"
#include "ui/UIPanel.h"
#include "utl/NetCacheLoader.h"
#include "utl/Symbol.h"
#include <list>

class StorePanel : public UIPanel {
public:
    // Hmx::Object
    virtual ~StorePanel();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Load();

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual bool Exiting() const;
    virtual bool Unloading() const; //
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual bool IsSongInLibrary(int const &) const = 0;
    virtual void ExitStore(StoreError) const = 0;
    virtual Profile *StoreProfile() const = 0;
    virtual StoreOffer *MakeNewOffer(DataArray *) = 0;
    virtual StoreOffer *FindOffer(Symbol) const = 0;
    virtual bool EnumerateSubsetOfOfferIDs() const { return 0; };
    virtual void
    GetOfferIDsToEnumerate(std::vector<unsigned long long> &, bool) const = 0;
    virtual void LoadArt(char const *, UIPanel *);

    StorePanel();
    void CheckOut(StorePurchaseable *);
    void ExitError(StoreError);
    void HandleNetCacheMgrFailure();
    void HandleNetCacheLoaderFailure(int);
    void MultipleItemsCheckout(std::list<StoreOffer *> *);

    std::vector<StoreOffer *> unk38;
    std::vector<StoreOffer *> unk44;
    bool unk50;
    bool mLoadOk; // 0x51
    bool unk52;
    std::list<NetCacheLoader *> unk54;
    int unk5c;
    RndTex *unk60;
    UIPanel *mPendingArtCallback; // 0x64
    int unk68;
    StorePreviewMgr *mStorePreviewMgr; // 0x6c
    bool unk70;
    StorePurchaser *mPurchaser; // 0x74
    StorePurchaseable *unk78;
    int unk7c;
    std::vector<StoreOffer *> unk80;
    Symbol unk8c;
    Symbol unk90;
    int unk94;
    Job *unk98;

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
    // DataNode __cdecl OnMsg(SingleItemEnumCompleteMsg const &);
    // DataNode __cdecl OnMsg(MultipleItemsEnumCompleteMsg const &);
};

class StoreEnumJob : public Job {
public:
    StoreEnumJob(StorePanel *, int, std::vector<UINT64> *);
    virtual ~StoreEnumJob();
    virtual bool IsFinished();
    virtual void OnCompletion(Hmx::Object *);

protected:
    XboxEnumeration *mEnumeration; // 0x8
    StorePanel *mStorePanel; // 0xc
};
