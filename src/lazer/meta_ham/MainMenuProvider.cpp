#include "meta_ham/MainMenuProvider.h"
#include "Challenges.h"
#include "meta_ham/AppLabel.h"
#include "os/Debug.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

MainMenuProvider::MainMenuProvider() {}

MainMenuProvider::~MainMenuProvider() {}

void MainMenuProvider::Text(
    int, int data, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT_RANGE(data, 0, mItems.size(), 0x2b);
    static Symbol challenges("challenges");
    static Symbol store("store");
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x30);
    Symbol item = mItems[data];
    if (uiListLabel->Matches("item")) {
        app_label->SetTextToken(item);
    } else if (uiListLabel->Matches("new")) {
        if (item == challenges) {
            app_label->SetNew(TheChallenges->HasNewChallenges());
        } else if (item == store) {
            app_label->SetNew(HasNewDLC());
        }
    }
}

int MainMenuProvider::NumData() const { return mItems.size(); }

int MainMenuProvider::DataIndex(Symbol s) const {
    for (int i = 0; i < mItems.size(); i++) {
        if (mItems[i] == s)
            return i;
    }
    return -1;
}

Symbol MainMenuProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mItems.size(), 0x6a);
    return mItems[idx];
}

void MainMenuProvider::UpdateList(UIListProvider *provider) {
    mItems.clear();
    for (int i = 0; i < provider->NumData(); i++) {
        mItems.push_back(provider->DataSymbol(i));
    }
}
