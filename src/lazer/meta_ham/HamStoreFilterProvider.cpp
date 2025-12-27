#include "meta_ham/HamStoreFilterProvider.h"
#include "meta_ham/AppLabel.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

HamStoreFilter::HamStoreFilter(Symbol s) {
    unk0 = s;
    unk4 = Localize(unk0, false, TheLocale);
}

HamStoreFilter::HamStoreFilter(DataArray const *d) {
    unk0 = d->Sym(0);
    static Symbol name("name");
    Symbol s = name; // why
    DataArray *nameArray = d->FindArray(s);
    unk4 = nameArray->Str(1);
    static Symbol sorts("sorts");
    DataArray *sortArray = d->FindArray(sorts);
    for (int i = 1; i < sortArray->Size(); i++) {
        unkc.push_back(sortArray->Sym(i));
    }
}

HamStoreFilterProvider::HamStoreFilterProvider(std::vector<HamStoreFilter *> *filters)
    : mFilters(filters) {}

Symbol HamStoreFilterProvider::DataSymbol(int data) const {
    MILO_ASSERT_RANGE(data, 0, mFilters->size(), 0x45);
    return (*mFilters)[data]->unk0;
}

int HamStoreFilterProvider::NumData() const { return mFilters->size(); }

void HamStoreFilterProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mFilters->size(), 0x2d);
    if (uiListLabel->Matches("label")) {
        static_cast<AppLabel *>(uiLabel)->SetStoreFilterName((*mFilters)[data]);
    } else {
        uiLabel->SetTextToken(gNullStr);
    }
}
