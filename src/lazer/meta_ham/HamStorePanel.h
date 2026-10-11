#pragma once
#include "meta/Profile.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "meta/StorePanel.h"
#include "meta/StorePurchaser.h"
#include "meta_ham/HamStoreFilterProvider.h"
#include "meta_ham/HamStoreProvider.h"
#include "net_ham/HamStoreCartJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Timer.h"
#include "os/User.h"
#include "types.h"
#include "utl/JobMgr.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include <list>

struct HamSpecialOffer {
public:
    Symbol mID; // 0x4
    unsigned long long mOfferId; // 0x8
    Symbol mLocalFileName; // 0x10
    bool mIsOwned; // 0x14
};

class HamStorePanel : public StorePanel, public ContentMgr::Callback {
public:
    // Hmx::Object
    virtual ~HamStorePanel();
    OBJ_CLASSNAME(HamStorePanel)
    OBJ_SET_TYPE(HamStorePanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Load();
    virtual bool Exiting() const;
    virtual void Poll();
    virtual void Unload();
    virtual bool IsSongInLibrary(int const &) const;
    virtual void ExitStore(StoreError) const;
    virtual Profile *StoreProfile() const;
    virtual StoreOffer *MakeNewOffer(DataArray *);
    virtual StoreOffer *FindOffer(Symbol) const;
    virtual bool EnumerateSubsetOfOfferIDs() const { return 0; }
    virtual void GetOfferIDsToEnumerate(std::vector<unsigned long long> &, bool) const;

    // ContentMgr::Callback
    virtual bool ContentDiscovered(Symbol);
    virtual bool ContentTitleDiscovered(unsigned int, Symbol);
    virtual void ContentMounted(char const *, char const *);

    NEW_OBJ(HamStorePanel)

    HamStorePanel();
    void LockCart();
    void UnlockCart();
    void EmptyCart();
    bool IsCurrFilterCart(int);
    void SetFilterToCart();
    int SetFilterToSongs();
    void RemoveDLCFromCart(int);
    void RemoveOfferFromCart(StoreOffer *);
    void AddOfferToCart(StoreOffer *);

protected:
    virtual StoreError UpdateOffers(std::list<EnumProduct> const &, bool);
    virtual void StoreUserProfileSwappedToUser(LocalUser *);

    void ReadLockData();
    void DisableCart();
    char const *GetIndexFile() const;
    void RefreshSpecialOfferStatus();
    void GetCart();
    void RelockCart();
    bool IsSpecialOfferOwned(Symbol) const;
    bool BuySpecialOffer(Symbol);
    void FinishSpecialOfferEnum(std::vector<bool> const &, bool);
    void RemoveNextDLCFromCart();
    void AddNextDLCToCart();
    void AddDLCToCart(int);
    void CreateCartUIs();
    void ReadCartData();
    void ResetCancelTimer();
    DataNode OnMsg(RCJobCompleteMsg const &);

private:
    DataNetLoader *mMetadataLoader; // 0xa0
    DataArray *mMetadata; // 0xa4
    HamStoreProvider *mOfferProvider; // 0xa8
    std::vector<HamStoreFilter *> mFilters; // 0xac
    String mMotd; // 0xb8
    bool mAllowCancel; // 0xc0
    Timer mCancelTimer; // 0xc4
    Timer mReLockCartTimer; // 0xf8
    int mReLockDuration; // 0x128
    std::vector<CartRow> mCartData; // 0x12c
    RCJob *mCartJobs[7]; // 0x138
    /*
        0x138 - [0] - AddDLCToCartJob
        0x13c - [1] - RemoveDLCFromCartJob
        0x140 - [2] - EmptyCartJob
        0x144 - [3] - GetCartJob
        0x148 - [4] - LockCartJob
        0x14c - [5] - UnlockCartJob
        0x150 - [6] - LockCartJob (for Relocking?)
    */
    bool mReadyToEnumCartOffer; // 0x154
    bool mIsCartEnabled; // 0x155- not in pdb
    bool mCartLocked; // 0x156 - not in pdb
    bool mCartLoaded; // 0x157 - not in pdb
    bool mRemovingDLCFromCart; // 0x158
    bool mAddingDLCFromCart; // 0x159
    std::list<int> mPendingRemovals; // 0x15c
    std::list<int> mPendingAdds; // 0x164
    std::vector<HamSpecialOffer> mSpecialOffers; // 0x16c
    std::vector<unsigned long long> mSpecialOfferIDs; // 0x178
    int mSpecialEnumJobID; // 0x184
    StorePurchaser *mSpecialPurchaser; // 0x188
};

class SpecialOfferEnumJob : public MultipleItemsEnumJob {
public:
    SpecialOfferEnumJob(HamStorePanel *panel, int i, std::vector<QWORD> &vec);
    virtual ~SpecialOfferEnumJob() {}
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);

protected:
    HamStorePanel *mStorePanel; // 0x5c - not in pdb
};
