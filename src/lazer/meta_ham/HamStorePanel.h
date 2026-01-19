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
#include "stl/_vector.h"
#include "types.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

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
    virtual void GetOfferIDsToEnumerate(std::vector<u64> &, bool) const;

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
    DataNode OnMsg(RCJobCompleteMsg const &);

    int unka0;
    int unka4;
    HamStoreProvider *mOfferProvider; // 0xa8
    std::vector<HamStoreFilter *> unkac;
    String unkb8;
    bool unkc0;
    Timer unkc4;
    Timer unkf8;
    int unk128;
    std::vector<CartRow> unk12c;
    RCJob *unk138[7];
    bool unk154;
    bool unk155;
    bool unk156;
    bool unk157;
    bool unk158;
    bool unk159;
    std::list<int> unk15c;
    std::list<int> unk164;
    std::vector<CartRow> unk16c;
    std::vector<unsigned long long> unk178;
    int unk184;
    XboxPurchaser *mXboxPurchaser; // 0x188
};
