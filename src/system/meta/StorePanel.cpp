#include "meta/StorePanel.h"
#include "meta/StoreEnumeration.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "ui/UIPanel.h"

StorePanel::StorePanel() {}

StorePanel::~StorePanel() {}

bool StorePanel::SyncProperty(DataNode &, DataArray *, int, PropOp) { return false; }

void StorePanel::Load() {}

void StorePanel::Enter() {}

void StorePanel::Exit() {}

bool StorePanel::Exiting() const { return false; }

void StorePanel::Poll() {}

bool StorePanel::IsLoaded() const { return false; }

void StorePanel::Unload() {}

void StorePanel::LoadArt(char const *, UIPanel *) {}

void StorePanel::CheckOut(StorePurchaseable *) {}

void StorePanel::ExitError(StoreError) {}

void StorePanel::HandleNetCacheLoaderFailure(int) {}

void StorePanel::MultipleItemsCheckout(std::list<StoreOffer *> *) {}

void StorePanel::PopulateOffers(DataArray *, bool) {}

void StorePanel::EnumerateOffers(bool) {}

void StorePanel::FinishEnum(std::list<EnumProduct> const &, bool) {}

StoreError StorePanel::UpdateOffers(std::list<EnumProduct> const &, bool) {
    return kStoreErrorNoMetadata;
}

void StorePanel::UpdateFromEnumProduct(StorePurchaseable *, EnumProduct const *) {}

void StorePanel::StartReEnum() {}

DataNode StorePanel::OnMsg(SigninChangedMsg const &) { return 0; }

void StorePanel::ValidateOffers(std::vector<StoreOffer *> &) {}
