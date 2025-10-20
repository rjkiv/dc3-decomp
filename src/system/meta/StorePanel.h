#pragma once
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "stl/_vector.h"
#include "ui/UIPanel.h"
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
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
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
    bool unk51;
    bool unk52;
    std::list<StorePanel *> unk54;
    int unk5c;
    Hmx::Object unk60;
    int unk64;
    int unk68;
    int unk6c;
    bool unk70;
    int unk74;
    int unk78;
    int unk7c;
    int unk80;
    int unk84;
    int unk88;
    Symbol unk8c;
    Symbol unk90;
    int unk94;
    int unk98;

protected:
    // UIPanel
    virtual void PopulateOffers(DataArray *, bool);
    virtual void EnumerateOffers(bool);
    virtual void FinishEnum(std::list<EnumProduct> const &, bool);
    virtual StoreError UpdateOffers(std::list<EnumProduct> const &, bool);
    virtual void UpdateFromEnumProduct(StorePurchaseable *, EnumProduct const *);

    void StartReEnum();
    DataNode OnMsg(SigninChangedMsg const &);
    void ValidateOffers(std::vector<StoreOffer *> &);
    // DataNode __cdecl OnMsg(SingleItemEnumCompleteMsg const &);
    // DataNode __cdecl OnMsg(MultipleItemsEnumCompleteMsg const &);
};
